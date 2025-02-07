#include <iostream>
#include <map>
#include <string>
#include <unordered_map>
using namespace std;

class Person {
   private:
    std::string name;
    int age;

   public:
    Person() : name(), age(0) {}

    // Constructor that takes string by value
    Person(std::string n, int a) : name(std::move(n)), age(a) {}

    // Optional: Copy constructor
    Person(const Person& other) : name(other.name), age(other.age) {}

    // Optional: Move constructor
    Person(Person&& other) noexcept : name(std::move(other.name)), age(other.age) {}

    // Add copy assignment operator
    Person& operator=(const Person& other) {
        name = other.name;
        age = other.age;
        return *this;
    }

    // Add move assignment operator
    Person& operator=(Person&& other) noexcept {
        name = std::move(other.name);
        age = other.age;
        return *this;
    }

    // Methods to access data
    const std::string& getName() const { return name; }
    int getAge() const { return age; }
};

int main() {
    cout << "Sample maps applications" << endl;

    map<int, string> intToStringMap;

    intToStringMap[0] = "Nirmesh";
    intToStringMap[1] = "Khandelwal";

    // idiomatic way to insert into map
    intToStringMap.insert(std::make_pair(2, "sharma"));

    // try inserting on the same map again,for existing key
    auto ret = intToStringMap.insert(std::make_pair(0, "sharma"));

    if (ret.second == false) {
        std::cout << "element '0' already existed";
        std::cout << " with a value of " << ret.first->second << '\n';
    }

    map<string, Person> map2;

    // Least efficient - creates and copies objects
    map2["key"] = Person("John", 25);
    map2.insert({"key", Person("John", 25)});

    // Better - constructs object in-place
    map2.emplace("key", Person("John", 25));

    // Best - C++17 - most efficient and prevents duplicates
    map2.try_emplace("key", "John", 25);  // constructs Person directly with args

    /** Iterate through everything  **/

    for (auto i = intToStringMap.begin(); i != intToStringMap.end(); i++) {
        cout << "Key: " << i->first << " Value: " << i->second << endl;
    }

    for (map<int, string>::iterator i = intToStringMap.begin(); i != intToStringMap.end(); i++) {
        cout << "Key: " << i->first << " Value: " << i->second << endl;
    }

    /** Update the value of an existing key **/

    auto iter = intToStringMap.find(0);

    if (iter != intToStringMap.end()) {
        iter->second = "NIRMESH";
    }

    for (auto i = intToStringMap.begin(); i != intToStringMap.end(); i++) {
        cout << "Key: " << i->first << " Value: " << i->second << endl;
    }

    cout << "Easier way to define iterators" << endl;

    for (auto& x : intToStringMap) {
        std::cout << x.first << ": " << x.second << '\n';
    }

    // check if key exists in the map

    iter = intToStringMap.find(2);

    if (iter != intToStringMap.end()) {
        // element found;
        cout << "element found, key: " << iter->first << " value:" << iter->second << endl;
    } else {
        cout << "element not found" << endl;
    }

    iter = intToStringMap.find(0);

    if (iter != intToStringMap.end()) {
        // element found;
        cout << "element found, key: " << iter->first << " value:" << iter->second << endl;
    } else {
        cout << "element not found" << endl;
    }

    // access the value directly for the key

    auto iter2 = intToStringMap.find(0);
    cout << "Key :" << iter2->first << " Value: " << iter2->second << endl;

    /* unordered_map */
    cout << endl << "======= unordered_map ======= " << endl;
    unordered_map<int, int> umap;
    umap[1] = 100;
    umap[100] = 1000;
    umap.insert(std::make_pair(129, 29));

    for (auto& it : umap) {
        cout << it.first << " : " << it.second << endl;
    }

    cout << endl << "Another way to iterate " << endl;

    for (auto it = umap.begin(); it != umap.end(); it++) {
        cout << it->first << " : " << it->second << endl;
    }

    return 0;
}
