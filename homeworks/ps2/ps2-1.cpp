//
// Created by Zeyu Zhang on 9/9/24.
//
#include <iostream>
#include <string>
#include <cstdlib>
#include "randshuffle.h"

using namespace std;

const string FOODS[] = { "Pizza from Mineo’s",
                               "Tofu Tikka Masala from Prince of India",
                               "Crispy Salmon Roll from Sushi Fuku",
                               "Sub from Uncle Sam’s",
                               "Fried rice from How Lee",
                               "Sandwiches from La Prima",
                               "Find free food on campus" };
const string DAYS[] = { "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun" };

int main() {
    int foodIndex[] = {0, 1, 2, 3, 4, 5, 6};
    randShuffle(foodIndex, 7);
    for (int i = 0; i < 7; i++) {
        cout << DAYS[i] << ": " << FOODS[foodIndex[i]] << endl;
    }
}