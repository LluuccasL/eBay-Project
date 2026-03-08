#include "marketplace.h"
#include <iostream>

using namespace std;


int main()
{

    Marketplace market;

    // Automatically generate 10 users
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

    // Create initial marketplace listings
    int laptop = market.createListing(1,"Laptop",600,900,300);
    int card = market.createListing(2,"Speaker",50,200,300);
    int phone = market.createListing(3,"Phone",120,350,300);

    while(true)
    {

        cout << "========= MARKETPLACE =========";

        market.showMarketplace();

        cout << "Options:" << endl;
        cout << "1 - Place Bid" << endl;
        cout << "2 - Add to Watchlist" << endl;
        cout << "3 - View Watchlist" << endl;
        cout << "4 - View History" << endl;
        cout << "5 - Exit" << endl;

        int choice;
        cout << "Enter option: ";
        cin >> choice;

	if(choice == 1)
        {
            int userID;
            int listingID;
            double bid;

            cout << "User ID (1-10): ";
            cin >> userID;

            cout << "Listing ID: ";
            cin >> listingID;

            cout << "Bid amount: ";
            cin >> bid;

            market.placeBid(userID,listingID,bid);
        }
	else if(choice == 2)
        {
            int userID;
            int listingID;

            cout << "User ID (1-10): ";
            cin >> userID;

            cout << "Listing ID to watch: ";
            cin >> listingID;

            market.addToWatchlist(userID,listingID);
        }
	else if(choice == 3)
        {
            int userID;

            cout << "User ID: ";
            cin >> userID;

            market.showUserWatchlist(userID);
        }
	else if(choice == 4)
        {
            int userID;

            cout << "User ID: ";
            cin >> userID;

            market.showUserHistory(userID);
        }
	else if(choice == 5)
        {
            cout << "Exiting marketplace..." << endl;
            break;
        }
	// check for auctions that have expired
        market.closeExpiredAuctions();
    }
    return 0;
}
