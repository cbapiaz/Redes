#ifndef UTIL_H
#define UTIL_H

#include <vector>
#include <string>

using namespace std;

class splitstring : public string {
    vector<string> flds;
public:
    splitstring(char *s) : string(s) { };
    splitstring(string s) : string(s) { };
    vector<string>& split(char delim, int rep);
};


#endif
