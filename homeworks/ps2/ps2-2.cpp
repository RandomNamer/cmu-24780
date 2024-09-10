//
// Created by Zeyu Zhang on 9/9/24.
//

//#define TEST_CARD_DISP
//#define USE_STD_PAIR_FOR_CARD
#define FORCE_5by7

# include <iostream>
#include <ctime>
#include <utility>
#include <array>
#include <cassert>
#include "randshuffle.h"

using namespace std;

void displaySingleFlashcard(int operand1, int operand2, unsigned int verticalPadding = 1, unsigned int paddingLeft = 1, unsigned int paddingRight = 1) {
    auto height = 3 + 2 * verticalPadding;
    assert(operand1 < 100 && operand2 < 100);
    int width = static_cast<int>(operand1 > 9) + static_cast<int>(operand2 > 9) + 5 + paddingLeft + paddingRight; // 12x3 would be 1 + 5 + all padding
    for (int i=0; i<height; i++) {
        if (i==0 || i==height-1) {
            cout<<'+';
            for (int j=1; j<width-1; j++) {
                cout << "-";
            }
            cout << '+';
        } else if (i == height / 2) {
            cout << '|';
            for (int j=0; j<paddingLeft; j++) {
                cout << ' ';
            }
            cout << operand1 <<'x'<<operand2;
            for (int j=0; j<paddingRight; j++) {
                cout << ' ';
            }
            cout << '|';
        } else {
            cout<<'|';
            for (int j=1; j<width-1; j++) {
                cout << " ";
            }
            cout << '|';
        }
        cout<<endl;
    }
}

void displayFlashcard5by7(int operand1, int operand2) {
    auto paddingLeft = static_cast<int>(operand1 < 10 && operand2 < 10);
    auto paddingRight = static_cast<int>(!(operand1 > 9 && operand2 > 9));
    return displaySingleFlashcard(operand1, operand2, 1, paddingLeft, paddingRight);
}

bool singleFlashcardFlow(int operand1, int operand2) {
#ifdef FORCE_5by7
    displayFlashcard5by7(operand1, operand2);
#else
    displaySingleFlashcard(operand1, operand2);
#endif
    cout << "Enter Your Answer>";
    int answer;
    cin >> answer;
    int solution = operand1 * operand2;
    if (answer == solution) {
        cout << "Correct!" << endl;
        return true;
    } else {
        cout << "Wrong. Correct answer is " << solution << endl;
        return false;
    }
}

template<size_t DIM>
std::array<std::pair<int, int>, DIM*DIM> initFlashcardSequence() {
    std::array<std::pair<int, int>, DIM*DIM> flashcardSequence;
    for (int i = 0; i < DIM; i++) {
        for (int j = 0; j < DIM; j++) {
            flashcardSequence[i * DIM + j] = std::make_pair(i+1, j+1);
        }
    }
    randShuffle(flashcardSequence, DIM*DIM);
    return std::move(flashcardSequence);
}

template<size_t DIM>
std::array<int, DIM*DIM> initFlashcardSequencePacked() {
    array<int, DIM*DIM> flashcardSequence;
    for (int i = 0; i < DIM; i++) {
        for (int j = 0; j < DIM; j++) {
            flashcardSequence[i * DIM + j] = (i+1) * 100 + (j+1);
        }
    }
    randShuffle(flashcardSequence, DIM*DIM);
    return std::move(flashcardSequence);
}

int main() {
#ifdef TEST_CARD_DISP
    displaySingleFlashcard(10, 15, 2, 2, 2);
    displaySingleFlashcard(11,4, 5, 1, 4);
    displaySingleFlashcard(114, 514, 3, 4);
    return 1;
#else
    int cardCount;
    FLOW_START:
    cout<<"Enter the number of flashcards you want to display: ";
    cin>>cardCount;
//    if(cin.fail()) {
//        cout<<"input malformed"<<endl;
//        cardCount = -114514;
//        cin.clear();
//    }
    if (cardCount < 1 || cardCount > 144) {
        cout << "The number of cards must be between 1 and 144" << endl;
        goto FLOW_START;
    }
    auto start = time(nullptr);
    srand(time(nullptr));
    int correctCount = 0;
#ifdef USE_STD_PAIR_FOR_CARD
    auto flashcardSequence = initFlashcardSequence<12>();
    for (int i = 0; i < cardCount; i++) {
        if (singleFlashcardFlow(flashcardSequence[i].first, flashcardSequence[i].second)) correctCount++;
    }
#else
    auto flashcardSequence = initFlashcardSequencePacked<12>();
    for (int i = 0; i < cardCount; i++) {
        if (singleFlashcardFlow(flashcardSequence[i] / 100, flashcardSequence[i] % 100)) correctCount++;
    }
#endif
    cout<<"You answered "<<cardCount<<" problems in "<<time(nullptr) - start<<
        " seconds. You answered "<<correctCount<<" problems correctly ("<<round((static_cast<float>(correctCount) / cardCount) * 100)<<"%)."<<endl;
    return 0;
#endif
}