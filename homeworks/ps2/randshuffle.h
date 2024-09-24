//
// Created by Zeyu Zhang on 9/9/24.
//

#ifndef HOMEWORKS_RANDSHUFFLE_H
#define HOMEWORKS_RANDSHUFFLE_H

#include <ctime>
#include <utility>

template <typename T>
void randShuffle(T& collection, size_t n) {
    //ignore [] operator check and swappable check, it's doable but too tedious
    srand(time(nullptr));
    for (size_t i = 0; i < n; i++) {
        size_t swapTarget = rand() % n;
        if (swapTarget != i) std::swap(collection[i], collection[swapTarget]);
    }
}

#endif //HOMEWORKS_RANDSHUFFLE_H
