#include <iostream>
#include <map>
using namespace std;


int main()  {
    cout << "Sample maps applications" << endl;

    map<int, string> intToStringMap;

    intToStringMap[0] = "Nirmesh";
    intToStringMap[1] = "Khandelwal";

    // idiomatic way to insert into map
    intToStringMap.insert(std::make_pair(2, "sharma"));
    

    // try inserting on the same map again,for existing key
    auto ret = intToStringMap.insert(std::make_pair(0, "sharma"));

    if (ret.second==false) {
        std::cout << "element '0' already existed";
        std::cout << " with a value of " << ret.first->second << '\n';
    }


    for(auto i = intToStringMap.begin(); i != intToStringMap.end(); i++) {
        cout << "Key: " << i->first <<" Value: " << i->second << endl;
    }

    /* Update the value of an existing key */

    auto iter = intToStringMap.find(0);

    if (iter != intToStringMap.end()) {
        iter->second = "NIRMESH";
    }

    for(auto i = intToStringMap.begin(); i != intToStringMap.end(); i++) {
        cout << "Key: " << i->first <<" Value: " << i->second << endl;
    }

    cout << "easier way to define iterators" << endl;

    for (auto& x: intToStringMap) {
        std::cout << x.first << ": " << x.second << '\n';
    }

    // check if key exists in the map

    iter = intToStringMap.find(2);
    
    if(iter != intToStringMap.end()) {
        //element found;
        cout << "element found, key: " << iter->first << " value:" << iter->second <<endl;
    } else {
        cout << "element not found" <<endl;
    }

    iter = intToStringMap.find(0); 

    if(iter != intToStringMap.end()) {
        //element found;
        cout << "element found, key: " << iter->first << " value:" << iter->second <<endl;
    } else {
        cout << "element not found" <<endl;
    }

    // access the value directly for the key

    auto iter2 = intToStringMap.find(0);
    cout << "Key :" << iter2->first << " Value: " << iter2->second <<endl;


    return 0;
}