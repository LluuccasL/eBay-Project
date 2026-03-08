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
Listing::Listing(int id, int seller, string item, double start, double buyNow, int duration)
{
    listingID = id;
    sellerID = seller;
    itemName = item;

    startingPrice = start;
    buyNowPrice = buyNow;

    // Auction expiration time
    endTime = time(nullptr) + duration;

    active = true;
}

void Listing::placeBid(int bidderID, double amount)
{

    // Reject bid if auction closed
    if (!active)
        return;

    // Reject bid if not higher than current price
    if (amount <= getCurrentPrice())
        return;

    // Insert bid into priority queue
    bids.push(Bid(bidderID, amount));

    // If buy-now reached, close auction immediately
    if (amount >= buyNowPrice)
        active = false;
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


bool Listing::isActive()
{

    // If auction time expired mark inactive
    if (active && time(nullptr) >= endTime)
        active = false;

    return active;
}


void Listing::closeAuction()
{
    active = false;
}


int Listing::getID() const
{
    return listingID;
}

string Listing::getName() const
{
    return itemName;
}

//User Implementation
User::User(int id, string n)
{
    userID = id;
    name = n;
}


int User::getID()
{
    return userID;
}


string User::getName()
{
    return name;
}


void User::addToWatchlist(int listingID)
{

    // Add listing to user's watchlist
    watchlist.push_back(listingID);
}

bool User::isWatching(int listingID)
{

    // Check if listing exists in watchlist
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
}


void Marketplace::addUser(int id, string name)
{

    // Create and store new user
    users.push_back(User(id, name));
}


User* Marketplace::findUser(int id)
{

    for (auto &u : users)
        if (u.getID() == id)
            return &u;

    return nullptr;
}


Listing* Marketplace::findListing(int id)
{

    for (auto &l : listings)
        if (l.getID() == id)
            return &l;

    return nullptr;
}

int Marketplace::createListing(int sellerID, string item, double startPrice, double buyNowPrice, int duration)
{

    // Create new listing object
    Listing newListing(nextListingID, sellerID, item, startPrice, buyNowPrice, duration);

    // Store listing
    listings.push_back(newListing);

    nextListingID++;

    return nextListingID - 1;
}


void Marketplace::placeBid(int bidderID, int listingID, double amount)
{

    Listing* listing = findListing(listingID);

    if (!listing)
        return;

    listing->placeBid(bidderID, amount);
}

void Marketplace::addToWatchlist(int userID, int listingID)
{

    User* user = findUser(userID);

    if (!user)
        return;

    user->addToWatchlist(listingID);
}

void Marketplace::showMarketplace()
{

    // Priority queue to sort listings by highest bid
    priority_queue<Listing*, vector<Listing*>, ListingCompare> marketplaceQueue;


    for (auto &l : listings)
        if (l.isActive())
            marketplaceQueue.push(&l);


    cout << "\nMarketplace (Highest Bid First)\n";


    while (!marketplaceQueue.empty())
    {

        Listing* l = marketplaceQueue.top();
        marketplaceQueue.pop();

        cout << "ID: " << l->getID()
             << " | " << l->getName()
             << " | Price: $" << l->getCurrentPrice()
             << " | Highest Bidder: " << l->getHighestBidder()
             << endl;
    }
}

void Marketplace::closeExpiredAuctions()
{

    for (auto &l : listings)
    {

        if (!l.isActive())
        {

            int winner = l.getHighestBidder();
            double price = l.getCurrentPrice();


            for (auto &u : users)
            {

                if (u.isWatching(l.getID()))
                {

                    bool won = (u.getID() == winner);

                    u.addHistory(AuctionResult(
                        l.getID(),
                        l.getName(),
                        price,
                        won
                    ));
                }
            }
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
