#include "marketplace.h"
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cmath>

using namespace std;

int main()
{
    srand(static_cast<unsigned>(time(nullptr)));
    Marketplace market;

    for (int i = 1; i <= 10; i++)
        market.addUser(i, "User" + to_string(i));

    const int numListings = 1000;
    const int bidsPerListing = 5;
    clock_t t0 = clock();

    for (int i = 0; i < numListings; i++)
    {
        int seller = 1 + (i % 10);
        market.createListing(seller, "Item" + to_string(i), "Category" + to_string(i % 20),
                            10.0 + (i % 100), 200.0 + (i % 500), 3600 + (i % 7200));
    }

    for (int round = 0; round < 3; round++)
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

    // Invariant checks
    auto all = market.getAllListings();
    int activeCount = 0, soldCount = 0, expiredCount = 0;
    for (const auto& l : all)
    {
        if (l.status == ListingStatus::ACTIVE) activeCount++;
        else if (l.status == ListingStatus::SOLD) soldCount++;
        else expiredCount++;
    }
    cout << "Listings: " << all.size() << " (active=" << activeCount << " sold=" << soldCount << " expired=" << expiredCount << ")\n";
    cout << "Stress test: " << numListings << " listings, " << (numListings * bidsPerListing * 3) << " bids\n";
    if (sec > 0)
        cout << "Throughput: " << (numListings / sec) << " listings/sec, " << (numListings * bidsPerListing * 3 / sec) << " bids/sec\n";
    cout << "Done.\n";
    return 0;
}
