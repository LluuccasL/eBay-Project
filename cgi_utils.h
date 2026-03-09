#ifndef CGI_UTILS_H
#define CGI_UTILS_H

#include <string>
#include <map>

namespace cgi {

// Get request method: "GET" or "POST"
std::string getRequestMethod();

// Get raw query string (GET params)
std::string getQueryString();

// Get raw POST body (call only for POST). Content-Length used.
std::string getPostBody();

// Parse application/x-www-form-urlencoded into key -> value map. Decodes both keys and values.
std::map<std::string, std::string> parseFormUrlEncoded(const std::string& body);

// Parse GET params from QUERY_STRING
std::map<std::string, std::string> parseQueryString();

// Get all CGI params: for GET use QUERY_STRING; for POST read stdin up to CONTENT_LENGTH.
std::map<std::string, std::string> getParams();

// URL-decode a string (replace %XX and +)
std::string urlDecode(const std::string& encoded);

// HTML-escape for safe output: & < > " '
std::string htmlEscape(const std::string& s);

} // namespace cgi

#endif
