#ifndef MARKETPLACE_H
#define MARKETPLACE_H

#include <vector>
#include <string>
#include <queue>
#include <deque>
#include <unordered_map>
#include <unordered_set>
#include <ctime>

// Bid Class
// Represents a single bid placed by a user on a listing
class Bid
{
public:
    int bidderID;      // ID of the user placing the bid
    double amount;     // Bid amount
    time_t timeStamp;  // Time when the bid was placed

    // Constructor
    Bid(int id, double price);
    // For persistence load: set timestamp explicitly
    Bid(int id, double price, time_t timestamp);

    // Comparator for priority_queue
    // Higher bid amount gets higher priority
    bool operator<(const Bid& other) const;
};

// AuctionResult Class
// Stores the result of a completed auction
class AuctionResult
{
public:
    int listingID;       // Listing identifier
    std::string itemName;// Name of item
    double finalPrice;   // Final auction price
    bool won;            // Whether the user won the auction

    // Constructor
    AuctionResult(int id, std::string name, double price, bool result);
};

// Listing status for display and filtering
enum class ListingStatus { ACTIVE, SOLD, EXPIRED };

// Listing Class
// Represents an item listed for auction
class Listing
{
private:

    int listingID;          // Unique listing ID
    int sellerID;           // Seller who created listing
    std::string itemName;   // Name of item
    std::string category;   // Listing category

    double startingPrice;   // Starting bid price
    double buyNowPrice;     // Buy-now instant purchase price

    time_t endTime;         // Time when auction ends (simulated or wall clock)

    bool active;            // Whether auction is still active
    ListingStatus status;   // ACTIVE, SOLD, or EXPIRED

    // Priority queue storing bids (highest at top)
    std::priority_queue<Bid> bids;
    // Copy of all bids for persistence (same bids as in queue)
    std::vector<Bid> allBids;
    // Lightweight set of bidder IDs for history when auction closes
    std::unordered_set<int> bidderIDs;

public:

    // Constructor: endTime is set by Marketplace (simClock + duration)
    Listing(int id, int seller, std::string item, std::string cat, double start, double buyNow, time_t endTime);

    // Places a new bid; returns true if accepted
    bool placeBid(int bidderID, double amount);

    // Returns current highest bid price
    double getCurrentPrice() const;

    // Returns ID of highest bidder
    int getHighestBidder() const;

    // Checks whether auction is still active at given time (use sim clock for simulation)
    bool isActive(time_t currentTime);
    // Const predicate: true if still active at currentTime (does not mutate)
    bool isActiveAt(time_t currentTime) const;

    // Manually closes auction
    void closeAuction();

    // Getter for listing ID
    int getID() const;

    // Getter for item name
    std::string getName() const;

    std::string getCategory() const;
    ListingStatus getStatus() const;
    time_t getEndTime() const;
    int getSellerID() const;
    double getStartingPrice() const;
    double getBuyNowPrice() const;
    // Returns copy of bidder IDs for history recording
    std::vector<int> getBidderIDs() const;
    // For persistence: add a bid without validation (used when loading state)
    void addBidForLoad(int bidderID, double amount, time_t timestamp);
    // For persistence: set active/status when loading (no other side effects)
    void setStateForLoad(bool isActive, ListingStatus st);
    // For persistence: return all bids (read-only)
    const std::vector<Bid>& getBidsRef() const { return allBids; }
};

// User Class
// Represents a user of the marketplace
class User
{
private:

    int userID;           // Unique user identifier
    std::string name;     // Username

    // Watchlist containing listing IDs
    std::deque<int> watchlist;

    // History of auctions the user participated in
    std::deque<AuctionResult> history;

public:

    // Constructor
    User(int id, std::string n);

    // Returns user ID
    int getID() const;

    // Returns username
    std::string getName() const;

    // Adds a listing to the watchlist
    void addToWatchlist(int listingID);

    // Removes a listing from the watchlist
    void removeFromWatchlist(int listingID);

    // Checks if user is watching a listing
    bool isWatching(int listingID) const;

    // Adds a completed auction to history
    void addHistory(AuctionResult result);

    // Displays watchlist
    void showWatchlist();

    // Displays auction history
    void showHistory();

    // Data accessors for CGI (return const refs to internal storage)
    const std::deque<int>& getWatchlistRef() const;
    const std::deque<AuctionResult>& getHistoryRef() const;
};

// ListingCompare Class
// Comparator used to sort listings in the marketplace
class ListingCompare
{
public:

    // Sort listings by highest current bid
    bool operator()(Listing* a, Listing* b);
};

// Transaction log entry for site-wide events
struct TransLogEntry
{
    std::string type;     // e.g. "user_created", "listing_created", "bid_placed", "auction_closed"
    std::string message;  // Human-readable or payload
    time_t when;          // Simulated or wall-clock time
};

// Listing info for CGI accessors (no pointers into internal storage)
struct ListingInfo
{
    int id;
    int sellerID;
    std::string itemName;
    std::string category;
    double startingPrice;
    double buyNowPrice;
    double currentPrice;
    int highestBidder;
    time_t endTime;
    ListingStatus status;
};

// Marketplace Class
// Central controller that manages users, listings, bids
class Marketplace
{
private:

    std::vector<User> users;
    std::vector<Listing> listings;
    int nextListingID;

    time_t simClock;   // Simulated time for auction expiration
    std::deque<TransLogEntry> transactionLog;  // O(1) append

    std::unordered_map<int, size_t> userIndex;     // userID -> index in users
    std::unordered_map<int, size_t> listingIndex; // listingID -> index in listings

    void logEvent(const std::string& type, const std::string& message);
    const User* findUserConst(int id) const;
    const Listing* findListingConst(int id) const;

public:

    Marketplace();

    void addUser(int id, std::string name);
    User* findUser(int id);
    Listing* findListing(int id);

    // Create listing; duration is in seconds from current sim time
    int createListing(int sellerID, std::string item, std::string category, double startPrice, double buyNowPrice, int duration);

    // Place bid; returns true if accepted
    bool placeBid(int bidderID, int listingID, double amount);

    void addToWatchlist(int userID, int listingID);
    void removeFromWatchlist(int userID, int listingID);

    void showMarketplace();
    // Uses simClock for expiration check; records history for bidders and watchers
    void closeExpiredAuctions();

    void showUserWatchlist(int userID);
    void showUserHistory(int userID);

    // Simulated time
    time_t getSimClock() const;
    void advanceTime(int seconds);

    // Data accessors for CGI (no stdout)
    std::vector<ListingInfo> getActiveListings() const;
    std::vector<ListingInfo> getAllListings() const;
    std::vector<int> getUserWatchlist(int userID) const;
    std::vector<AuctionResult> getUserHistory(int userID) const;
    std::vector<TransLogEntry> getTransactionLogEntries() const;

    // Persistence for CGI: state file path set by env MARKETPLACE_STATE or default
    bool loadState(const std::string& path);
    void saveState(const std::string& path) const;
};

#endif
