# Contributors

This document attributes the work in this repo: the original codebase (Lucas) and the extensions added after cloning (Krishna).

---

## Lucas’s work (original repo)

**Source:** [LluuccasL/eBay-Project](https://github.com/LluuccasL/eBay-Project) (cloned as the starting point).

### Core design and types

- **Bid** — `bidderID`, `amount`, `timeStamp`; `operator<` for max-heap (highest bid on top).
- **AuctionResult** — `listingID`, `itemName`, `finalPrice`, `won` for user history.
- **Listing** — `listingID`, `sellerID`, `itemName`, `startingPrice`, `buyNowPrice`, `endTime`, `active`, `priority_queue<Bid> bids`; methods: `placeBid`, `getCurrentPrice`, `getHighestBidder`, `isActive`, `closeAuction`, `getID`, `getName`. Expiration and buy-now (bid ≥ buyNowPrice closes auction).
- **User** — `userID`, `name`, `deque<int> watchlist`, `deque<AuctionResult> history`; `addToWatchlist`, `isWatching`, `addHistory`, `showWatchlist`, `showHistory`.
- **ListingCompare** — comparator to sort listings by highest current bid.
- **Marketplace** — `vector<User>`, `vector<Listing>`, `nextListingID`; `addUser`, `findUser`, `findListing`, `createListing`, `placeBid`, `addToWatchlist`, `showMarketplace`, `closeExpiredAuctions`, `showUserWatchlist`, `showUserHistory`.

### CLI (main.cxx)

- Program entry; creation of 10 users and 3 initial listings (Laptop, Speaker, Phone).
- Main loop with menu: Place Bid, Add to Watchlist, View Watchlist, View History, Exit.
- Calling `closeExpiredAuctions()` each iteration.

### Other

- **README.md** — initial one-line project title.

All of the above existed in the repo at clone time and is attributed to Lucas.

---

## Krishna’s work (after cloning)

**Branch:** `kc` (extensions built on top of Lucas’s repo).

### Backend (marketplace.h / marketplace.cxx)

- **Listing**
  - Added `category` (string) and `ListingStatus` enum (ACTIVE, SOLD, EXPIRED).
  - Constructor extended to take `category` and `endTime` (Marketplace sets `endTime = simClock + duration`).
  - `placeBid` now returns `bool`; rejects when bidder is the seller.
  - Added `bidderIDs` (`unordered_set<int>`) for history; `placeBid` registers bidders.
  - Added `isActive(time_t)` (mutating) and `isActiveAt(time_t)` const for expiration.
  - New getters: `getCategory`, `getStatus`, `getEndTime`, `getSellerID`, `getStartingPrice`, `getBuyNowPrice`, `getBidderIDs`.
- **User**
  - `removeFromWatchlist(int)`.
  - `getWatchlistRef()` and `getHistoryRef()` for CGI.
  - `getID()`, `getName()`, `isWatching()` made const where needed.
- **Marketplace**
  - Simulated time: `simClock`, `advanceTime(seconds)`; `closeExpiredAuctions()` uses `simClock`.
  - Transaction log: `transactionLog` (deque), `logEvent()`, logging for user/listing/bid/close/expire.
  - O(1) lookups: `userIndex`, `listingIndex` (unordered_maps); `findUser`/`findListing` use them; `findUserConst`/`findListingConst` for const access.
  - `createListing` extended with `category`; `placeBid` returns bool and logs.
  - `removeFromWatchlist(userID, listingID)`.
  - `closeExpiredAuctions()` updated to record history for both watchers and bidders (using `bidderIDs`).
  - Data accessors for CGI: `getActiveListings()`, `getAllListings()`, `getUserWatchlist()`, `getUserHistory()`, `getTransactionLogEntries()`.
  - Structs: `TransLogEntry`, `ListingInfo`.

### CLI (main.cxx)

- New menu options: Create Listing (6), Remove from Watchlist (3), Advance Time (7), Show Transaction Log (8); renumbered menu and Exit (9).
  - Create listing: seller, item, category, start price, buy-now price, duration.
  - Remove from watchlist: user ID, listing ID.
  - Advance time: seconds; then `closeExpiredAuctions()`.
  - Show transaction log: print log entries.
- `--stress`: when present, run in-process stress test then exit.
- Initial listings created with category `"Electronics"`.
  - Bid acceptance/rejection printed after place-bid.

### Build and stress test

- **Makefile** — targets: `marketplace_cli`, `marketplace_cgi`, `stress_test`.
- **stress_test.cxx** — standalone program: ~1000 listings, multiple bids per item, advance time, close auctions, invariant checks, throughput (listings/sec, bids/sec).

### CGI and web UI

- **cgi_utils.h** / **cgi_utils.cxx** — `getRequestMethod`, `getQueryString`, `getPostBody`, `parseFormUrlEncoded`, `getParams`, `urlDecode`, `htmlEscape`.
- **cgi_main.cxx** — single static Marketplace; ensures 10 users; dispatches on `action=` (listings, create_listing, place_bid, watch, unwatch, advance_time, logs, user_history); calls Marketplace only; renders HTML with escaped output.
- **static/index.html** — forms and links for all CGI actions (view listings, create listing, place bid, watch/unwatch, advance time, transaction log, user history).

### Documentation and audit

- **README.md** — build, run CLI, run CGI, open frontend, run stress test, example flow, CGI persistence note, project layout.
- **AUDIT.md** — repo audit: what existed (Lucas), what was partial, what was missing, files to extend, new files (used to plan Krishna’s work).

All of the above was implemented after cloning and is attributed to Krishna.

---

## Summary

| Area              | Lucas (original)                         | Krishna (extensions)                                      |
|-------------------|------------------------------------------|----------------------------------------------------------|
| Core types        | Bid, AuctionResult, Listing, User, etc.  | Category, status enum, bidder set, const helpers, getters |
| Marketplace       | Vectors, basic CRUD, show*               | Sim clock, transaction log, index maps, data accessors   |
| CLI               | 5 options (bid, watch, view watch/history, exit) | + create listing, remove watch, advance time, log, --stress |
| Build             | —                                        | Makefile, stress_test target                             |
| CGI + HTML        | —                                        | cgi_utils, cgi_main, static/index.html                   |
| Docs              | README title                             | README full, AUDIT.md, CONTRIBUTORS.md                   |
