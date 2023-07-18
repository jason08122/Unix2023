#include <iostream>
#include <dirent.h>
#include <sys/types.h>
#include <bits/stdc++.h>
using namespace std;



int main(int argc, char *argv[])
{
    ifstream ifs;
    DIR *dr;
    char buffer[256] = {0};
    struct dirent *en;
    string ans(argv[2]);
    //string tmp, num(argv[2]);
    //int magic = stoi(num);
    dr = opendir(argv[1]);
    if (dr)
    {
        while ((en = readdir(dr)) != NULL)
        {
            //cout<<" \n"<<en->d_name;
            //tmp  = en->d_name;
            if ( en->d_name == "." || en->d_name == ".." ) continue;
            ifs.open(en->d_name);

            if (!ifs.is_open())
            {
                //cout << "Failed to open file.\n";
                continue;
            }
            ifs.read(buffer, sizeof(buffer));
            string content(buffer);

            size_t found = content.find(ans);

             if (found != string::npos) cout<<en->d_name<<endl;
            //cout << "First occurrence is " <<found << endl;

            //if ( content.find(ans) ) cout<<"found"<<endl<<en->d_name<<endl;;
            ifs.close();
        }
        closedir(dr);
    }
    return(0);
}

