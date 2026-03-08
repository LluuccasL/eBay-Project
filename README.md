# eBay-Project (eBay-lite)

A simplified eBay-style marketplace in C++ with auctions, bids, watchlists, and user history.

**Contributors:** The original codebase is by **Lucas** ([LluuccasL/eBay-Project](https://github.com/LluuccasL/eBay-Project)); extensions (simulated time, transaction log, CGI, stress test, etc.) are by **Krishna**. See [CONTRIBUTORS.md](CONTRIBUTORS.md) for a detailed breakdown of what each person implemented.

## Build

```bash
make
```

This produces:

- **marketplace_cli** — interactive CLI
- **marketplace_cgi** — CGI executable for web UI
- **stress_test** — stress test executable

Requirements: C++17 compiler (e.g. `g++`).

## Run CLI

```bash
./marketplace_cli
```

Menu options:

1. Place Bid  
2. Add to Watchlist  
3. Remove from Watchlist  
4. View Watchlist  
5. View History  
6. Create Listing  
7. Advance Time  
8. Show Transaction Log  
9. Exit  

Users 1–10 are pre-created. Create listings (option 6), place bids (1), advance simulated time (7) to close auctions, then view history (5) or transaction log (8).

## Run stress test

```bash
./stress_test
```

Or via the CLI:

```bash
./marketplace_cli --stress
```

Creates ~1000 listings, places several bids per item, advances time, closes auctions, and prints throughput (listings/sec, bids/sec).

## Run CGI and open frontend

1. Build: `make`
2. Run a web server that executes the CGI binary for requests. For a quick local test you can use a small server or Python:

   ```bash
   cd static
   python3 -m http.server 8000
   ```

   Then configure your server so that requests to e.g. `marketplace_cgi` run the `marketplace_cgi` binary from the project root (or copy `marketplace_cgi` into a cgi-bin directory and point the form actions to it).

   Alternatively, with lighttpd you can set `cgi.assign` to run `marketplace_cgi` and serve static files from `static/`.

3. Open the frontend:  
   **http://localhost:8000/index.html**  
   (adjust host/port if your server differs)

4. From the static page you can:
   - **View active listings** — list of active auctions
   - **View transaction log** — site-wide event log
   - **Create listing** — seller ID, item, category, start price, buy-now price, duration
   - **Place bid** — user ID, listing ID, amount
   - **Add / Remove from watchlist** — user ID, listing ID
   - **Advance time** — seconds to advance (closes expired auctions)
   - **View user history** — won/lost auctions for a user

**CGI persistence:** Standard CGI runs a new process per request. This implementation does not persist state to disk, so each request starts with a fresh marketplace (10 users, no listings). To keep state across requests you would need to add a minimal persistence layer (e.g. load/save in Marketplace) or run under a long-lived process (e.g. FastCGI).

## Example flow (CLI)

1. Start: `./marketplace_cli`
2. Create a listing: option 6, e.g. seller 1, item "Laptop", category "Electronics", start 600, buy-now 900, duration 300.
3. Place a bid: option 1, user 2, listing ID from step 2, amount 650.
4. Add to watchlist: option 2, user 3, same listing ID.
5. Advance time: option 7, e.g. 400 seconds. Auctions that expire will close.
6. View history: option 5, user 2 or 3 — see WON/LOST and final price.
7. View transaction log: option 8.

## Project layout

- `marketplace.h` / `marketplace.cxx` — core types (Bid, Listing, User, Marketplace) and logic
- `main.cxx` — CLI entry point; `--stress` runs the in-process stress test
- `cgi_utils.h` / `cgi_utils.cxx` — CGI helpers (GET/POST, URL decode, HTML escape)
- `cgi_main.cxx` — CGI entry point; parses `action=` and calls Marketplace
- `static/index.html` — static forms/links for the web UI
- `stress_test.cxx` — standalone stress test program
- `AUDIT.md` — repo audit (what exists, what was added)
- `CONTRIBUTORS.md` — Lucas’s original work vs Krishna’s extensions
