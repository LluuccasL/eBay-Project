#include "marketplace.h"
#include "cgi_utils.h"
#include <iostream>
#include <sstream>
#include <cstdlib>

using namespace std;

// CGI runs one process per request; state is not persisted between requests.
// See README for persistence options.

static Marketplace& getMarket()
{
    static Marketplace market;
    return market;
}

static void ensureUsers()
{
    Marketplace& m = getMarket();
    if (m.findUser(1) != nullptr)
        return;
    for (int i = 1; i <= 10; i++)
        m.addUser(i, "User" + to_string(i));
}

static void printHeader(const string& title)
{
    cout << "Content-Type: text/html\r\n\r\n";
    cout << "<!DOCTYPE html><html><head><meta charset=\"UTF-8\"><title>"
         << cgi::htmlEscape(title) << "</title></head><body>\n";
    cout << "<h1>" << cgi::htmlEscape(title) << "</h1>\n";
}

static void printFooter()
{
    cout << "<p><a href=\"index.html\">Back to menu</a></p>\n";
    cout << "</body></html>\n";
}

int main()
{
    ensureUsers();
    Marketplace& m = getMarket();
    auto params = cgi::getParams();
    string action = params.count("action") ? params["action"] : "";

    if (action == "listings")
    {
        printHeader("Active Listings");
        auto list = m.getActiveListings();
        cout << "<p>Sim time: " << m.getSimClock() << "</p>\n";
        cout << "<table border=\"1\"><tr><th>ID</th><th>Item</th><th>Category</th><th>Current</th><th>Highest Bidder</th><th>End</th></tr>\n";
        for (const auto& l : list)
        {
            cout << "<tr><td>" << l.id << "</td><td>" << cgi::htmlEscape(l.itemName) << "</td><td>"
                 << cgi::htmlEscape(l.category) << "</td><td>$" << l.currentPrice << "</td><td>"
                 << l.highestBidder << "</td><td>" << l.endTime << "</td></tr>\n";
        }
        cout << "</table>\n";
        printFooter();
        return 0;
    }

    if (action == "create_listing")
    {
        printHeader("Create Listing");
        int sellerID = atoi(params.count("seller_id") ? params["seller_id"].c_str() : "1");
        string item = params.count("item") ? params["item"] : "";
        string category = params.count("category") ? params["category"] : "General";
        double start = atof(params.count("start_price") ? params["start_price"].c_str() : "0");
        double buyNow = atof(params.count("buy_now_price") ? params["buy_now_price"].c_str() : "0");
        int duration = atoi(params.count("duration") ? params["duration"].c_str() : "3600");
        if (!item.empty() && start > 0 && buyNow >= start && duration > 0)
        {
            int id = m.createListing(sellerID, item, category, start, buyNow, duration);
            cout << "<p>Created listing ID " << id << ".</p>\n";
        }
        else
            cout << "<p>Invalid input. Need item, start_price, buy_now_price, duration.</p>\n";
        printFooter();
        return 0;
    }

    if (action == "place_bid")
    {
        printHeader("Place Bid");
        int bidderID = atoi(params.count("user_id") ? params["user_id"].c_str() : "0");
        int listingID = atoi(params.count("listing_id") ? params["listing_id"].c_str() : "0");
        double amount = atof(params.count("amount") ? params["amount"].c_str() : "0");
        if (bidderID > 0 && listingID > 0 && amount > 0)
        {
            if (m.placeBid(bidderID, listingID, amount))
                cout << "<p>Bid accepted.</p>\n";
            else
                cout << "<p>Bid rejected (inactive, too low, or you are the seller).</p>\n";
        }
        else
            cout << "<p>Need user_id, listing_id, amount.</p>\n";
        printFooter();
        return 0;
    }

    if (action == "watch")
    {
        printHeader("Add to Watchlist");
        int userID = atoi(params.count("user_id") ? params["user_id"].c_str() : "0");
        int listingID = atoi(params.count("listing_id") ? params["listing_id"].c_str() : "0");
        if (userID > 0 && listingID > 0)
        {
            m.addToWatchlist(userID, listingID);
            cout << "<p>Added listing " << listingID << " to user " << userID << " watchlist.</p>\n";
        }
        printFooter();
        return 0;
    }

    if (action == "unwatch")
    {
        printHeader("Remove from Watchlist");
        int userID = atoi(params.count("user_id") ? params["user_id"].c_str() : "0");
        int listingID = atoi(params.count("listing_id") ? params["listing_id"].c_str() : "0");
        if (userID > 0 && listingID > 0)
        {
            m.removeFromWatchlist(userID, listingID);
            cout << "<p>Removed listing " << listingID << " from user " << userID << " watchlist.</p>\n";
        }
        printFooter();
        return 0;
    }

    if (action == "advance_time")
    {
        printHeader("Advance Time");
        int seconds = atoi(params.count("seconds") ? params["seconds"].c_str() : "60");
        if (seconds > 0)
        {
            m.advanceTime(seconds);
            m.closeExpiredAuctions();
            cout << "<p>Time advanced by " << seconds << "s. Sim clock: " << m.getSimClock() << "</p>\n";
        }
        printFooter();
        return 0;
    }

    if (action == "logs")
    {
        printHeader("Transaction Log");
        auto entries = m.getTransactionLogEntries();
        cout << "<table border=\"1\"><tr><th>Type</th><th>Message</th><th>Time</th></tr>\n";
        for (const auto& e : entries)
            cout << "<tr><td>" << cgi::htmlEscape(e.type) << "</td><td>" << cgi::htmlEscape(e.message)
                 << "</td><td>" << e.when << "</td></tr>\n";
        cout << "</table>\n";
        printFooter();
        return 0;
    }

    if (action == "user_history")
    {
        printHeader("User History");
        int userID = atoi(params.count("user_id") ? params["user_id"].c_str() : "0");
        if (userID > 0)
        {
            auto hist = m.getUserHistory(userID);
            cout << "<p>User " << userID << " history:</p><table border=\"1\"><tr><th>Item</th><th>Price</th><th>Result</th></tr>\n";
            for (const auto& h : hist)
                cout << "<tr><td>" << cgi::htmlEscape(h.itemName) << "</td><td>$" << h.finalPrice
                     << "</td><td>" << (h.won ? "WON" : "LOST") << "</td></tr>\n";
            cout << "</table>\n";
        }
        else
            cout << "<p>Need user_id.</p>\n";
        printFooter();
        return 0;
    }

    printHeader("eBay-lite CGI");
    cout << "<p>Use index.html to choose an action. Supported: action=listings|create_listing|place_bid|watch|unwatch|advance_time|logs|user_history</p>\n";
    printFooter();
    return 0;
}
