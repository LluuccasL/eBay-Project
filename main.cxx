#include "marketplace.h"
#include <iostream>
#include <string>
#include <cstdlib>
#include <ctime>

using namespace std;

static void runStressTest()
{
    Marketplace market;
    for (int i = 1; i <= 10; i++)
        market.addUser(i, "User" + to_string(i));

    const int numListings = 1000;
    const int bidsPerListing = 5;
    clock_t t0 = clock();
    for (int i = 0; i < numListings; i++)
    {
        int seller = 1 + (i % 10);
        market.createListing(seller, "Item" + to_string(i), "Category" + to_string(i % 20), 10.0 + (i % 100), 200.0 + (i % 500), 3600 + (i % 7200));
    }
    for (int step = 0; step < 3; step++)
    {
        for (int i = 0; i < numListings * bidsPerListing; i++)
        {
            int userID = 1 + (rand() % 10);
            int listingID = 1 + (rand() % numListings);
            double amount = 20.0 + (rand() % 500);
            market.placeBid(userID, listingID, amount);
        }
        market.advanceTime(4000);
        market.closeExpiredAuctions();
    }
    market.advanceTime(20000);
    market.closeExpiredAuctions();
    clock_t t1 = clock();
    double sec = (double)(t1 - t0) / CLOCKS_PER_SEC;
    cout << "Stress test: " << numListings << " listings, " << (numListings * bidsPerListing * 3) << " bids, "
         << (sec > 0 ? (numListings / sec) : 0) << " listings/sec, "
         << (sec > 0 ? (numListings * bidsPerListing * 3 / sec) : 0) << " bids/sec\n";
    cout << "Done.\n";
}

int main(int argc, char* argv[])
{
    if (argc >= 2 && string(argv[1]) == "--stress")
    {
        srand((unsigned)time(nullptr));
        runStressTest();
        return 0;
    }

    Marketplace market;

    market.addUser(1,"User1");
    market.addUser(2,"User2");
    market.addUser(3,"User3");
    market.addUser(4,"User4");
    market.addUser(5,"User5");
    market.addUser(6,"User6");
    market.addUser(7,"User7");
    market.addUser(8,"User8");
    market.addUser(9,"User9");
    market.addUser(10,"User10");

    market.createListing(1,"Laptop","Electronics",600,900,300);
    market.createListing(2,"Speaker","Electronics",50,200,200);
    market.createListing(3,"Table","Furniture",120,350,600);

    while(true)
    {
        cout << "========= MARKETPLACE =========";
        cout << " (Sim time: " << market.getSimClock() << ")\n";
        market.showMarketplace();

        cout << "Options:" << endl;
        cout << "1 - Place Bid" << endl;
        cout << "2 - Add to Watchlist" << endl;
        cout << "3 - Remove from Watchlist" << endl;
        cout << "4 - View Watchlist" << endl;
        cout << "5 - View History" << endl;
        cout << "6 - Create Listing" << endl;
        cout << "7 - Advance Time" << endl;
        cout << "8 - Show Transaction Log" << endl;
        cout << "9 - Exit" << endl;

        int choice;
        cout << "Enter option: ";
        cin >> choice;

        if (choice == 1)
        {
            int userID, listingID;
            double bid;
            cout << "User ID (1-10): ";
            cin >> userID;
            cout << "Listing ID: ";
            cin >> listingID;
            cout << "Bid amount: ";
            cin >> bid;
            if (market.placeBid(userID, listingID, bid))
                cout << "Bid accepted.\n";
            else
                cout << "Bid rejected (inactive, too low, or you are the seller).\n";
        }
        else if (choice == 2)
        {
            int userID, listingID;
            cout << "User ID (1-10): ";
            cin >> userID;
            cout << "Listing ID to watch: ";
            cin >> listingID;
            market.addToWatchlist(userID, listingID);
        }
        else if (choice == 3)
        {
            int userID, listingID;
            cout << "User ID (1-10): ";
            cin >> userID;
            cout << "Listing ID to remove from watchlist: ";
            cin >> listingID;
            market.removeFromWatchlist(userID, listingID);
        }
        else if (choice == 4)
        {
            int userID;
            cout << "User ID: ";
            cin >> userID;
            market.showUserWatchlist(userID);
        }
        else if (choice == 5)
        {
            int userID;
            cout << "User ID: ";
            cin >> userID;
            market.showUserHistory(userID);
        }
        else if (choice == 6)
        {
            int sellerID, duration;
            string item, category;
            double startPrice, buyNowPrice;
            cout << "Seller User ID (1-10): ";
            cin >> sellerID;
            cout << "Item name: ";
            cin >> item;
            cout << "Category: ";
            cin >> category;
            cout << "Starting price: ";
            cin >> startPrice;
            cout << "Buy-now price: ";
            cin >> buyNowPrice;
            cout << "Duration (seconds): ";
            cin >> duration;
            int id = market.createListing(sellerID, item, category, startPrice, buyNowPrice, duration);
            cout << "Created listing ID " << id << "\n";
        }
        else if (choice == 7)
        {
            int seconds;
            cout << "Seconds to advance: ";
            cin >> seconds;
            market.advanceTime(seconds);
            market.closeExpiredAuctions();
            cout << "Time advanced. Sim clock: " << market.getSimClock() << "\n";
        }
        else if (choice == 8)
        {
            auto entries = market.getTransactionLogEntries();
            cout << "Transaction log (" << entries.size() << " entries):\n";
            for (const auto& e : entries)
                cout << "  [" << e.type << "] " << e.message << " (t=" << e.when << ")\n";
        }
        else if (choice == 9)
        {
            cout << "Exiting marketplace..." << endl;
            break;
        }

        market.closeExpiredAuctions();
    }
    return 0;
}
