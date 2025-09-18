#include <algorithm>
#include <iostream>
#include <iterator>
#include <list>

using namespace std;

int main (int argc, char *argv[]) {

    list<int> l;
    for (int  i = 0; i < 3; i++) {
        l.push_back(i);
    }

    for (auto it = l.begin(); it != l.end(); ++it) {
        // cout << *it << " : " << distance(it, l.end()) << endl;
        cout << *it << " : " << distance(l.begin(), it) << endl;
    }
    
    return 0;
}
