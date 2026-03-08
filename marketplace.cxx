#include "marketplace.h"
#include <iostream>

using namespace std;

//Bid Implementation
Bid::Bid(int id, double price)
{
    bidderID = id;
    amount = price;

    // Record current time when bid is created
    timeStamp = time(nullptr);
}

bool Bid::operator<(const Bid& other) const
{
    // Higher bid amount should appear first in priority queue
    return amount < other.amount;
}

//Auction Implementation
AuctionResult::AuctionResult(int id, string name, double price, bool result)
{
    listingID = id;
    itemName = name;
    finalPrice = price;
    won = result;
}

//Listing Implementation
Listing::Listing(int id, int seller, string item, string cat, double start, double buyNow, time_t end)
{
    listingID = id;
    sellerID = seller;
    itemName = item;
    category = cat;
    startingPrice = start;
    buyNowPrice = buyNow;
    endTime = end;
    active = true;
    status = ListingStatus::ACTIVE;
}

bool Listing::placeBid(int bidderID, double amount)
{
    if (!active)
        return false;
    if (bidderID == sellerID)
        return false;
    if (amount <= getCurrentPrice())
        return false;

    bids.push(Bid(bidderID, amount));
    bidderIDs.insert(bidderID);

    if (amount >= buyNowPrice) {
        active = false;
        status = (bids.empty() ? ListingStatus::EXPIRED : ListingStatus::SOLD);
    }
    return true;
}


double Listing::getCurrentPrice() const
{

    // If no bids yet return starting price
    if (bids.empty())
        return startingPrice;

    // Otherwise return highest bid
    return bids.top().amount;
}

int Listing::getHighestBidder() const
{

    if (bids.empty())
        return -1;

    return bids.top().bidderID;
}


bool Listing::isActive(time_t currentTime)
{
    if (active && currentTime >= endTime) {
        active = false;
        status = (getHighestBidder() >= 0 ? ListingStatus::SOLD : ListingStatus::EXPIRED);
    }
    return active;
}

bool Listing::isActiveAt(time_t currentTime) const
{
    return active && currentTime < endTime;
}

void Listing::closeAuction()
{
    active = false;
    status = (getHighestBidder() >= 0 ? ListingStatus::SOLD : ListingStatus::EXPIRED);
}

int Listing::getID() const { return listingID; }
string Listing::getName() const { return itemName; }
string Listing::getCategory() const { return category; }
ListingStatus Listing::getStatus() const { return status; }
time_t Listing::getEndTime() const { return endTime; }
int Listing::getSellerID() const { return sellerID; }
double Listing::getStartingPrice() const { return startingPrice; }
double Listing::getBuyNowPrice() const { return buyNowPrice; }

vector<int> Listing::getBidderIDs() const
{
    vector<int> out(bidderIDs.begin(), bidderIDs.end());
    return out;
}

//User Implementation
User::User(int id, string n)
{
    userID = id;
    name = n;
}


int User::getID() const
{
    return userID;
}

string User::getName() const
{
    return name;
}


void User::addToWatchlist(int listingID)
{
    watchlist.push_back(listingID);
}

void User::removeFromWatchlist(int listingID)
{
    for (auto it = watchlist.begin(); it != watchlist.end(); ++it) {
        if (*it == listingID) {
            watchlist.erase(it);
            return;
        }
    }
}

bool User::isWatching(int listingID) const
{
    for (int id : watchlist)
        if (id == listingID)
            return true;
    return false;
}


void User::addHistory(AuctionResult result)
{

    // Store auction result
    history.push_back(result);
}


void User::showWatchlist()
{

    cout << "Watchlist:\n";

    for (int id : watchlist)
        cout << "Listing " << id << endl;
}

void User::showHistory()
{

    cout << "History:\n";

    for (auto &h : history)
    {
        cout << h.itemName
             << " | $" << h.finalPrice
             << " | " << (h.won ? "WON" : "LOST")
             << endl;
    }
}

const deque<int>& User::getWatchlistRef() const { return watchlist; }
const deque<AuctionResult>& User::getHistoryRef() const { return history; }

//ListingCompare Implementation
bool ListingCompare::operator()(Listing* a, Listing* b)
{

    // Sort by highest current price
    return a->getCurrentPrice() < b->getCurrentPrice();
}

//Marketplace Implementation
Marketplace::Marketplace()
{
    nextListingID = 1;
    simClock = time(nullptr);
}

void Marketplace::logEvent(const string& type, const string& message)
{
    transactionLog.push_back({ type, message, simClock });
}

void Marketplace::addUser(int id, string name)
{
    users.push_back(User(id, name));
    userIndex[id] = users.size() - 1;
    logEvent("user_created", "User " + to_string(id) + " " + name);
}

User* Marketplace::findUser(int id)
{
    auto it = userIndex.find(id);
    if (it != userIndex.end())
        return &users[it->second];
    return nullptr;
}

Listing* Marketplace::findListing(int id)
{
    auto it = listingIndex.find(id);
    if (it != listingIndex.end())
        return &listings[it->second];
    return nullptr;
}

const User* Marketplace::findUserConst(int id) const
{
    auto it = userIndex.find(id);
    if (it != userIndex.end())
        return &users[it->second];
    return nullptr;
}

const Listing* Marketplace::findListingConst(int id) const
{
    auto it = listingIndex.find(id);
    if (it != listingIndex.end())
        return &listings[it->second];
    return nullptr;
}

int Marketplace::createListing(int sellerID, string item, string category, double startPrice, double buyNowPrice, int duration)
{
    time_t endTime = simClock + duration;
    Listing newListing(nextListingID, sellerID, item, category, startPrice, buyNowPrice, endTime);
    listings.push_back(newListing);
    listingIndex[nextListingID] = listings.size() - 1;
    int id = nextListingID;
    nextListingID++;
    logEvent("listing_created", "Listing " + to_string(id) + " " + item + " (" + category + ")");
    return id;
}

bool Marketplace::placeBid(int bidderID, int listingID, double amount)
{
    Listing* listing = findListing(listingID);
    if (!listing)
        return false;
    bool ok = listing->placeBid(bidderID, amount);
    if (ok)
        logEvent("bid_placed", "User " + to_string(bidderID) + " bid " + to_string(amount) + " on listing " + to_string(listingID));
    return ok;
}

void Marketplace::addToWatchlist(int userID, int listingID)
{
    User* user = findUser(userID);
    if (!user)
        return;
    user->addToWatchlist(listingID);
}

void Marketplace::removeFromWatchlist(int userID, int listingID)
{
    User* user = findUser(userID);
    if (!user)
        return;
    user->removeFromWatchlist(listingID);
}

void Marketplace::showMarketplace()
{
    priority_queue<Listing*, vector<Listing*>, ListingCompare> marketplaceQueue;
    for (auto &l : listings)
        if (l.isActive(simClock))
            marketplaceQueue.push(&l);

    cout << "\nMarketplace (Highest Bid First)\n";
    while (!marketplaceQueue.empty())
    {
        Listing* l = marketplaceQueue.top();
        marketplaceQueue.pop();
        cout << "ID: " << l->getID()
             << " | " << l->getName()
             << " | Category: " << l->getCategory()
             << " | Price: $" << l->getCurrentPrice()
             << " | Highest Bidder: " << l->getHighestBidder()
             << endl;
    }
}

void Marketplace::closeExpiredAuctions()
{
    for (auto &l : listings)
    {
        if (!l.isActive(simClock))
        {
            int winner = l.getHighestBidder();
            double price = l.getCurrentPrice();
            int lid = l.getID();
            string lname = l.getName();

            // Collect user IDs that should get history: watchers + bidders
            std::unordered_set<int> toNotify;
            for (const auto &u : users)
                if (u.isWatching(lid))
                    toNotify.insert(u.getID());
            for (int bidderId : l.getBidderIDs())
                toNotify.insert(bidderId);

            for (auto &u : users)
            {
                if (toNotify.count(u.getID()))
                {
                    bool won = (u.getID() == winner);
                    u.addHistory(AuctionResult(lid, lname, price, won));
                }
            }

            if (winner >= 0)
                logEvent("auction_closed", "Listing " + to_string(lid) + " sold to user " + to_string(winner) + " at " + to_string(price));
            else
                logEvent("auction_expired", "Listing " + to_string(lid) + " expired with no bids");
        }
    }
}

void Marketplace::showUserWatchlist(int userID)
{

    User* u = findUser(userID);

    if (u)
        u->showWatchlist();
}


void Marketplace::showUserHistory(int userID)
{
    User* u = findUser(userID);
    if (u)
        u->showHistory();
}

time_t Marketplace::getSimClock() const { return simClock; }

void Marketplace::advanceTime(int seconds)
{
    simClock += seconds;
}

vector<ListingInfo> Marketplace::getActiveListings() const
{
    vector<ListingInfo> out;
    for (const auto &l : listings)
    {
        if (l.isActiveAt(simClock))
        {
            ListingInfo info;
            info.id = l.getID();
            info.sellerID = l.getSellerID();
            info.itemName = l.getName();
            info.category = l.getCategory();
            info.startingPrice = l.getStartingPrice();
            info.buyNowPrice = l.getBuyNowPrice();
            info.currentPrice = l.getCurrentPrice();
            info.highestBidder = l.getHighestBidder();
            info.endTime = l.getEndTime();
            info.status = l.getStatus();
            out.push_back(info);
        }
    }
    return out;
}

vector<ListingInfo> Marketplace::getAllListings() const
{
    vector<ListingInfo> out;
    for (const auto &l : listings)
    {
        ListingInfo info;
        info.id = l.getID();
        info.sellerID = l.getSellerID();
        info.itemName = l.getName();
        info.category = l.getCategory();
        info.startingPrice = l.getStartingPrice();
        info.buyNowPrice = l.getBuyNowPrice();
        info.currentPrice = l.getCurrentPrice();
        info.highestBidder = l.getHighestBidder();
        info.endTime = l.getEndTime();
        info.status = l.getStatus();
        out.push_back(info);
    }
    return out;
}

vector<int> Marketplace::getUserWatchlist(int userID) const
{
    const User* u = findUserConst(userID);
    if (!u)
        return {};
    const auto& wl = u->getWatchlistRef();
    return vector<int>(wl.begin(), wl.end());
}

vector<AuctionResult> Marketplace::getUserHistory(int userID) const
{
    const User* u = findUserConst(userID);
    if (!u)
        return {};
    const auto& h = u->getHistoryRef();
    return vector<AuctionResult>(h.begin(), h.end());
}

vector<TransLogEntry> Marketplace::getTransactionLogEntries() const
{
    return vector<TransLogEntry>(transactionLog.begin(), transactionLog.end());
}
