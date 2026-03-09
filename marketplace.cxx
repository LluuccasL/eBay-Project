#include "marketplace.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <stdexcept>

using namespace std;

static vector<string> splitTabs(const string& line)
{
    vector<string> out;
    stringstream ss(line);
    string part;
    while (getline(ss, part, '\t'))
        out.push_back(part);
    return out;
}

static string escapeForState(const string& s)
{
    string out;
    for (char c : s) {
        if (c == '\t' || c == '\n') out += ' ';
        else out += c;
    }
    return out;
}

//Bid Implementation
Bid::Bid(int id, double price)
{
    bidderID = id;
    amount = price;
    timeStamp = time(nullptr);
}

Bid::Bid(int id, double price, time_t timestamp)
{
    bidderID = id;
    amount = price;
    timeStamp = timestamp;
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

    Bid b(bidderID, amount);
    bids.push(b);
    allBids.push_back(b);
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

void Listing::addBidForLoad(int bidderID, double amount, time_t timestamp)
{
    Bid b(bidderID, amount, timestamp);
    bids.push(b);
    allBids.push_back(b);
    bidderIDs.insert(bidderID);
}

void Listing::setStateForLoad(bool isActive, ListingStatus st)
{
    active = isActive;
    status = st;
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
    simClock = 0;
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

bool Marketplace::loadState(const string& path)
{
    ifstream f(path);
    if (!f)
        return false;
    users.clear();
    listings.clear();
    transactionLog.clear();
    userIndex.clear();
    listingIndex.clear();
    nextListingID = 1;
    simClock = 0;
    string line;
    while (getline(f, line))
    {
        if (line.empty()) continue;
        vector<string> t = splitTabs(line);
        if (t.empty()) continue;
        const string& tag = t[0];
        try {
        if (tag == "SIM_CLOCK" && t.size() >= 2) { simClock = (time_t)stoll(t[1]); continue; }
        if (tag == "NEXT_LISTING_ID" && t.size() >= 2) { nextListingID = stoi(t[1]); continue; }
        if (tag == "USER" && t.size() >= 3)
        {
            int id = stoi(t[1]);
            string name = t[2];
            if (name.empty()) name = "User" + to_string(id);
            users.push_back(User(id, name));
            userIndex[id] = users.size() - 1;
            continue;
        }
        if (tag == "LISTING" && t.size() >= 10)
        {
            int id = stoi(t[1]), sellerID = stoi(t[2]);
            string itemName = t[3], category = t[4];
            double start = stod(t[5]), buyNow = stod(t[6]);
            time_t endTime = (time_t)stoll(t[7]);
            int act = stoi(t[8]), st = stoi(t[9]);
            Listing L(id, sellerID, itemName, category, start, buyNow, endTime);
            L.setStateForLoad(act != 0, st == 0 ? ListingStatus::ACTIVE : (st == 1 ? ListingStatus::SOLD : ListingStatus::EXPIRED));
            listings.push_back(L);
            listingIndex[id] = listings.size() - 1;
            continue;
        }
        if (tag == "LISTING_BID" && t.size() >= 5)
        {
            int lid = stoi(t[1]), bidderID = stoi(t[2]);
            double amount = stod(t[3]);
            time_t ts = (time_t)stoll(t[4]);
            Listing* L = findListing(lid);
            if (L) L->addBidForLoad(bidderID, amount, ts);
            continue;
        }
        if (tag == "WATCHLIST" && t.size() >= 3)
        {
            int uid = stoi(t[1]), lid = stoi(t[2]);
            User* u = findUser(uid);
            if (u) u->addToWatchlist(lid);
            continue;
        }
        if (tag == "HISTORY" && t.size() >= 6)
        {
            int uid = stoi(t[1]), lid = stoi(t[2]);
            string itemName = t[3];
            double price = stod(t[4]);
            int won = stoi(t[5]);
            User* u = findUser(uid);
            if (u) u->addHistory(AuctionResult(lid, itemName, price, won != 0));
            continue;
        }
        if (tag == "LOG" && t.size() >= 4)
        {
            transactionLog.push_back({ t[1], t[2], (time_t)stoll(t[3]) });
            continue;
        }
        } catch (const std::exception&) { /* skip bad line */ }
    }
    return true;
}

void Marketplace::saveState(const string& path) const
{
    ofstream f(path);
    if (!f) return;
    f << "SIM_CLOCK\t" << simClock << "\n";
    f << "NEXT_LISTING_ID\t" << nextListingID << "\n";
    for (const auto& u : users)
        f << "USER\t" << u.getID() << "\t" << escapeForState(u.getName()) << "\n";
    for (const auto& l : listings)
    {
        int st = (l.getStatus() == ListingStatus::ACTIVE ? 0 : (l.getStatus() == ListingStatus::SOLD ? 1 : 2));
        int act = l.isActiveAt(simClock) ? 1 : 0;
        f << "LISTING\t" << l.getID() << "\t" << l.getSellerID() << "\t" << escapeForState(l.getName()) << "\t"
          << escapeForState(l.getCategory()) << "\t" << l.getStartingPrice() << "\t" << l.getBuyNowPrice() << "\t"
          << l.getEndTime() << "\t" << act << "\t" << st << "\n";
        for (const auto& b : l.getBidsRef())
            f << "LISTING_BID\t" << l.getID() << "\t" << b.bidderID << "\t" << b.amount << "\t" << b.timeStamp << "\n";
    }
    for (const auto& u : users)
        for (int lid : u.getWatchlistRef())
            f << "WATCHLIST\t" << u.getID() << "\t" << lid << "\n";
    for (const auto& u : users)
        for (const auto& h : u.getHistoryRef())
            f << "HISTORY\t" << u.getID() << "\t" << h.listingID << "\t" << escapeForState(h.itemName) << "\t" << h.finalPrice << "\t" << (h.won ? 1 : 0) << "\n";
    for (const auto& e : transactionLog)
        f << "LOG\t" << escapeForState(e.type) << "\t" << escapeForState(e.message) << "\t" << e.when << "\n";
}
