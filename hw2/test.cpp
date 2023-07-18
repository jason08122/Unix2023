#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstring>
#include <stdlib.h>

using namespace std;

int main() {
    ifstream file("/proc/1891659/maps");
    if (!file) {
        cout << "Failed to open the file." << endl;
        return 1;
    }

    vector<pair<unsigned long long, unsigned long long>> res;
    string line, a, b;
    while (getline(file, line)) {
        if (line.find("/hello") != string::npos) {
            cout<<line<<endl;
            int n = size(line), idx = 0;
            string head, tail;
            for ( int i=0;i<n;i++)
            {
                if (line[i] == '-') 
                {
                    head = line.substr(0, i);
                    idx = i;
                }
                if (line[i] == ' ')
                {
                    tail = line.substr(idx+1, i - idx-1);
                    break;
                }
            }
            // cout<<head<<endl<<tail<<endl;
            unsigned long long a = stoull(head, nullptr, 16);
            unsigned long long b = stoull(tail, nullptr, 16);
            res.push_back({a, b});
        }
    }

    for ( auto it:res) 
    {
        cout<<it.first<<" "<<it.second<<endl;
    }

    file.close();
    return 0;
}




