# eBay-Project Repo Audit

## What Already Exists

| Area | Details |
|------|--------|
| **Bid** | `bidderID`, `amount`, `timeStamp`; `operator<` for max-heap (higher amount = higher priority). |
| **AuctionResult** | `listingID`, `itemName`, `finalPrice`, `won` — used for user history. |
| **Listing** | `listingID`, `sellerID`, `itemName`, `startingPrice`, `buyNowPrice`, `endTime`, `active`, `priority_queue<Bid> bids`. Methods: `placeBid`, `getCurrentPrice`, `getHighestBidder`, `isActive`, `closeAuction`, `getID`, `getName`. Uses `time(nullptr)` for `endTime` and in `isActive()`. Buy-now closes auction when bid >= buyNowPrice. |
| **User** | `userID`, `name`, `deque<int> watchlist`, `deque<AuctionResult> history`. `addToWatchlist`, `isWatching`, `addHistory`, `showWatchlist`, `showHistory`. |
| **ListingCompare** | Sorts by `getCurrentPrice()` (highest first). |
| **Marketplace** | `vector<User> users`, `vector<Listing> listings`, `nextListingID`. `addUser`, `findUser`, `findListing`, `createListing`, `placeBid`, `addToWatchlist`, `showMarketplace`, `closeExpiredAuctions`, `showUserWatchlist`, `showUserHistory`. |
| **main.cxx** | Creates 10 users, 3 initial listings; loop with menu: Place Bid, Add to Watchlist, View Watchlist, View History, Exit. Calls `closeExpiredAuctions()` each iteration. |

## What Is Partially Implemented

- **Bid validation**: `Listing::placeBid` rejects if auction inactive or `amount <= getCurrentPrice()`. It does **not** reject when bidder is the seller.
- **History on close**: `closeExpiredAuctions()` only adds `AuctionResult` to users who **watch** the listing (`isWatching`). Users who **bid** but did not watch never get history.
- **Time**: Uses wall-clock `time(nullptr)`; no simulated clock or `advanceTime`.
- **Lookups**: `findUser` and `findListing` are O(n) over vectors; no index maps.

## What Is Missing

- **Listing**: No `category`; no explicit status enum (ACTIVE/SOLD/EXPIRED).
- **Watchlist**: No `removeFromWatchlist` in User or Marketplace.
- **Bidder history**: When an auction closes, bidders (not only watchers) must receive history; requires tracking bidders per listing without breaking the existing `priority_queue` (e.g. `unordered_set<int> bidderIDs` or `vector<Bid> allBids`).
- **Simulated time**: No `advanceTime(seconds)`, no simulation clock; `closeExpiredAuctions()` should use simulated time.
- **Transaction log**: No site-wide chronological log (user created, listing created, bid placed, buy-now, auction closed/expired) with O(1) append and retrieval API.
- **Fast lookup**: No O(1) user (and optionally listing) index; extend with `unordered_map` over existing vectors.
- **CGI-oriented accessors**: No data-returning APIs; only `show*` that print to stdout. Need: `getActiveListings()`, `getAllListings()`, `getUserWatchlist(userID)`, `getUserHistory(userID)`, `getTransactionLogEntries()` (and possibly getters for Listing/User for HTML rendering).
- **CLI**: No create listing, remove from watchlist, advance time, transaction log, or stress-test mode (e.g. `--stress`).
- **CGI + HTML**: No CGI binary, no `cgi_utils`, no static HTML with forms/actions.
- **Build**: No Makefile or CMakeLists.txt.
- **Stress test**: No dedicated stress test (~1000 items, multiple bids, time advance, invariants, throughput).
- **README**: Minimal; needs build, run CLI/CGI, frontend, stress test, and example flow.

## Files to Extend (No Duplicates)

- **marketplace.h** — Listing (category, status enum), User (removeFromWatchlist), Marketplace (sim clock, advanceTime, transaction log, index maps, data accessors, removeFromWatchlist, bidder tracking for history). Add struct/class for log entries and return types for accessors (e.g. `vector` of simple structs).
- **marketplace.cxx** — Implement all of the above; `closeExpiredAuctions()` using sim time and recording history for bidders; keep `priority_queue` for top bid, add lightweight bidder set/vector per listing.
- **main.cxx** — Add menu: create listing, remove from watchlist, view user history (already present as "View History"), advance time, show transaction log; parse `--stress` and run stress test (or delegate to separate executable).
- **README.md** — Build, run CLI, run CGI, open frontend, stress test, example flow.

## New Files (Only Where Needed)

- **AUDIT.md** — This audit.
- **cgi_utils.h** / **cgi_utils.cxx** — GET/POST parsing, URL decode, HTML escape, env (REQUEST_METHOD, QUERY_STRING, CONTENT_LENGTH).
- **cgi_main.cxx** — Parse `action=...`, call Marketplace methods, render HTML; no business logic.
- **static/index.html** — Forms/buttons: view listings, create listing, place bid, watch, unwatch, advance time, transaction log, user history; actions as specified.
- **Makefile** or **CMakeLists.txt** — Targets: CLI app, CGI executable, stress-test executable.
- **stress_test.cxx** — Create ~1000 items, multiple bids per item, advance time, close auctions, check invariants and throughput.
