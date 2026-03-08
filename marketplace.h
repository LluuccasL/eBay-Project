#ifndef MARKETPLACE_H
#define MARKETPLACE_H

#include <vector>
#include <string>
#include <queue>
#include <deque>
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

// Listing Class
// Represents an item listed for auction
class Listing
{
private:

    int listingID;          // Unique listing ID
    int sellerID;           // Seller who created listing
    std::string itemName;   // Name of item

    double startingPrice;   // Starting bid price
    double buyNowPrice;     // Buy-now instant purchase price

    time_t endTime;         // Time when auction ends

    bool active;            // Whether auction is still active

    // Priority queue storing bids
    // Highest bid always stays at the top
    std::priority_queue<Bid> bids;

public:

    // Constructor
    Listing(int id, int seller, std::string item, double start, double buyNow, int duration);

    // Places a new bid on the listing
    void placeBid(int bidderID, double amount);

    // Returns current highest bid price
    double getCurrentPrice() const;

    // Returns ID of highest bidder
    int getHighestBidder() const;

    // Checks whether auction is still active
    bool isActive();

    // Manually closes auction
    void closeAuction();

    // Getter for listing ID
    int getID() const;

    // Getter for item name
    std::string getName() const;
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
    int getID();

    // Returns username
    std::string getName();

    // Adds a listing to the watchlist
    void addToWatchlist(int listingID);

    // Checks if user is watching a listing
    bool isWatching(int listingID);

    // Adds a completed auction to history
    void addHistory(AuctionResult result);

    // Displays watchlist
    void showWatchlist();

    // Displays auction history
    void showHistory();
};

// ListingCompare Class
// Comparator used to sort listings in the marketplace
class ListingCompare
{
public:

    // Sort listings by highest current bid
    bool operator()(Listing* a, Listing* b);
};

// Marketplace Class
// Central controller that manages users, listings, bids
class Marketplace
{
private:

    // List of users
    std::vector<User> users;

    // List of marketplace listings
    std::vector<Listing> listings;

    // Used to assign unique listing IDs
    int nextListingID;

public:

    // Constructor
    Marketplace();

    // Adds a new user
    void addUser(int id, std::string name);

    // Finds a user by ID
    User* findUser(int id);

    // Finds a listing by ID
    Listing* findListing(int id);

    // Creates a new marketplace listing
    int createListing(int sellerID, std::string item, double startPrice, double buyNowPrice, int duration);

    // Places a bid on a listing
    void placeBid(int bidderID, int listingID, double amount);

    // Adds listing to a user's watchlist
    void addToWatchlist(int userID, int listingID);

    // Displays marketplace listings
    void showMarketplace();

    // Closes expired auctions and records results
    void closeExpiredAuctions();

    // Displays watchlist for a specific user
    void showUserWatchlist(int userID);

    // Displays auction history for a user
    void showUserHistory(int userID);
};

#endif
