//
// Created by Zeyu Zhang on 8/28/24.
//

//#define ALTERNATE_SINGLE_LINE_INPUT;

#include <iostream>
#include <string>
#include<vector>

using namespace std;

const static string PROMPT = "ax+by=c\ndx+ey=f\nEnter a b c d e f:";

vector<std::string> split(string s, string delimiter) {
    size_t pos_start = 0, pos_end, delim_len = delimiter.length();
    string token;
    vector<std::string> res;

    while ((pos_end = s.find(delimiter, pos_start)) != std::string::npos) {
        token = s.substr (pos_start, pos_end - pos_start);
        pos_start = pos_end + delim_len;
        res.push_back (token);
    }

    res.push_back (s.substr (pos_start));
    return res;
}

void takeInputsSplitBySpace(double& a, double& b, double& c, double& d, double& e, double& f) {
    string inputLine;
    cin>>inputLine;
    vector<string> tokens = split(inputLine, " ");
    cout<<tokens.size();
    if (tokens.size() >= 5) {
        a = stod(tokens[0]);
        b = stod(tokens[1]);
        c = stod(tokens[2]);
        d = stod(tokens[3]);
        e = stod(tokens[4]);
        f = stod(tokens[5]);
    } else {
        cout<<"Not enough numbers\n";
    }
}

int main() {
    double a,b,c,d,e,f;
    setbuf(stdout, 0);
    cout<<PROMPT;
#ifdef ALTERNATE_SINGLE_LINE_INPUT
    takeInputsSplitBySpace(a,b,c,d,e,f);
#else
    cin>>a>>b>>c>>d>>e>>f;
#endif
    auto base = a*e - b*d;
    if (abs(base) > 0.000001) {
        auto x = (e*c - b*f) / base;
        auto y = (a*f - c*d) / base;
        cout<<"x="<<x<<" y="<<y;
    } else cout<<"No Solution";
}

