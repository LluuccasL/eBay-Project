#!/usr/bin/env python3
"""
Serves static files from static/ and runs the C++ CGI binary (marketplace_cgi)
for paths starting with /marketplace_cgi. Run from project root:
  python3 run_cgi_server.py [port]
Then open http://localhost:8000/
"""
import os
import subprocess
import sys
from http.server import HTTPServer, BaseHTTPRequestHandler

PORT = 8000
PROJECT_ROOT = os.path.dirname(os.path.abspath(__file__))
STATIC_DIR = os.path.join(PROJECT_ROOT, "static")
CGI_BIN = os.path.join(PROJECT_ROOT, "marketplace_cgi")


class CGIHandler(BaseHTTPRequestHandler):
    def do_GET(self):
        path = self.path.split("?")[0]
        query = self.path.split("?", 1)[1] if "?" in self.path else ""
        if path == "/marketplace_cgi" or path.rstrip("/") == "/marketplace_cgi":
            self.run_cgi(query, None)
            return
        self.serve_static(path)

    def do_POST(self):
        path = self.path.split("?")[0]
        query = self.path.split("?", 1)[1] if "?" in self.path else ""
        body = None
        if "content-length" in [h.lower() for h in self.headers]:
            n = int(self.headers.get("Content-Length", 0))
            if 0 < n <= 1024 * 1024:
                body = self.rfile.read(n)
        if path == "/marketplace_cgi" or path.rstrip("/") == "/marketplace_cgi":
            self.run_cgi(query, body)
            return
        self.serve_static(path)

    def run_cgi(self, query_string, post_body):
        if not os.path.isfile(CGI_BIN):
            self.send_error(502, "marketplace_cgi not built. Run: make")
            return
        env = os.environ.copy()
        env["REQUEST_METHOD"] = "POST" if post_body is not None else "GET"
        env["QUERY_STRING"] = query_string
        if post_body is not None:
            env["CONTENT_LENGTH"] = str(len(post_body))
            env["CONTENT_TYPE"] = self.headers.get("Content-Type", "application/x-www-form-urlencoded")
        try:
            result = subprocess.run(
                [CGI_BIN],
                input=post_body,
                capture_output=True,
                cwd=PROJECT_ROOT,
                env=env,
                timeout=10,
            )
            out = result.stdout.decode("utf-8", errors="replace")
            err = result.stderr.decode("utf-8", errors="replace") if result.stderr else ""
            if result.returncode != 0:
                self.send_response(502)
                self.send_header("Content-Type", "text/html; charset=utf-8")
                self.end_headers()
                msg = "CGI exited with code %d." % result.returncode
                if err:
                    msg += " <pre>%s</pre>" % err.replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;")
                self.wfile.write(("<html><body><h1>Error</h1><p>%s</p><p><a href='/'>Back to menu</a></p></body></html>" % msg).encode("utf-8"))
                return
            if "Content-Type:" in out:
                head, body = out, ""
                for sep in ("\r\n\r\n", "\n\n"):
                    if sep in out:
                        head, _, body = out.partition(sep)
                        break
                head = head.replace("\r", "")
                self.send_response(200)
                for line in head.split("\n"):
                    if ":" in line:
                        k, v = line.split(":", 1)
                        self.send_header(k.strip(), v.strip())
                self.end_headers()
                self.wfile.write(body.encode("utf-8", errors="replace"))
            else:
                self.send_response(200)
                self.send_header("Content-Type", "text/html; charset=utf-8")
                self.end_headers()
                self.wfile.write(out.encode("utf-8", errors="replace"))
        except subprocess.TimeoutExpired:
            self.send_error(504, "CGI timeout")
        except Exception as e:
            self.send_error(502, str(e))

    def serve_static(self, path):
        if path in ("/", ""):
            path = "/index.html"
        if path.startswith("/"):
            path = path[1:]
        filepath = os.path.normpath(os.path.join(STATIC_DIR, path))
        if not filepath.startswith(STATIC_DIR) or ".." in path:
            self.send_error(404, "Not Found")
            return
        if not os.path.isfile(filepath):
            self.send_error(404, "Not Found")
            return
        ext = os.path.splitext(filepath)[1].lower()
        types = {".html": "text/html", ".css": "text/css", ".js": "application/javascript"}
        with open(filepath, "rb") as f:
            self.send_response(200)
            self.send_header("Content-Type", types.get(ext, "application/octet-stream"))
            self.send_header("Content-Length", os.path.getsize(filepath))
            self.end_headers()
            self.wfile.write(f.read())

    def log_message(self, format, *args):
        print("[%s] %s" % (self.log_date_time_string(), format % args))


def main():
    port = int(sys.argv[1]) if len(sys.argv) > 1 else PORT
    server = HTTPServer(("", port), CGIHandler)
    print("eBay-lite: open http://localhost:%d/" % port)
    print("Static: %s  CGI: %s" % (STATIC_DIR, CGI_BIN))
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        print("\nShutting down.")
        server.shutdown()


if __name__ == "__main__":
    main()
