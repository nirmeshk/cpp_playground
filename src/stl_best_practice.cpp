#include <algorithm>
#include <iostream>
#include <list>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

// Example class for container operations
class Person {
   private:
    string name;
    int age;

   public:
    Person() : name(), age(0) {}

    Person(string n, int a) : name(std::move(n)), age(a) {}

    Person(const Person& other) : name(other.name), age(other.age) {}

    Person(Person&& other) noexcept : name(std::move(other.name)), age(other.age) {}

    Person& operator=(const Person& other) {
        name = other.name;
        age = other.age;
        return *this;
    }

    Person& operator=(Person&& other) noexcept {
        name = std::move(other.name);
        age = other.age;
        return *this;
    }

    const string& getName() const { return name; }
    int getAge() const { return age; }
};


// Vector Best Practices
void vectorBestPractices() {
    cout << "\n=== Vector Best Practices ===\n";

    // 1. Capacity Management
    vector<string> names;
    names.reserve(1000);
    for (int i = 0; i < 1000; i++) {
        names.emplace_back("name" + to_string(i));
    }

    // only necessary if you really need to release the extra allocated memory.
    // It can be an expensive operation,
    // so don't use it unless memory is a critical concern.
    names.shrink_to_fit();

    // 2. Efficient Removal
    vector<int> nums = {1, 2, 3, 4, 5, 2, 3, 2};

    // remove(nums.begin(), nums.end(), 2)
    // remove just moves the elements to the end instead of re-sizing everything
    // It will still do moves, but avoid memory allocations
    // This way, if you have multiple removals, you can avoid multiple
    // re-shrinkings
    nums.erase(remove(nums.begin(), nums.end(), 2), nums.end());

    // 3. Efficient Insertion
    vector<pair<string, int>> pairs;
    pairs.reserve(100);
    for (int i = 0; i < 100; i++) {
        pairs.emplace_back("item" + to_string(i), i);
    }
}

// Map Best Practices
void mapBestPractices() {
    cout << "\n=== Map Best Practices ===\n";

    map<string, Person> people;

    // 1. Efficient Insertion
    people.try_emplace("john", "John Doe", 30);

    // 2. Efficient Lookup and Insert
    auto [it, inserted] = people.try_emplace("jane", "Jane Doe", 25);
    if (!inserted) {
        cout << "Jane already exists\n";
    }

    // 3. Iteration
    for (const auto& [key, value] : people) {
        cout << key << ": " << value.getName() << "\n";
    }

    // Checking key exists 
    
    std::unordered_map<string, int> m;

    // DON'T do this to check existence:
    if (m["key"]) { }  // This will INSERT a key if it doesn't exist!

    // modern
    if (m.contains("key")) {
        // key exists
    }
    
    // modern
    if (m.count("key")) {
        // key exists
    }

    // most efficniet
    m["key"]++; // insert 0 if does not exists, and at the same time increment it

}



// List Best Practices
void listBestPractices() {
    cout << "\n=== List Best Practices ===\n";

    list<int> numbers;

    // 1. Efficient Insertion
    auto it = numbers.begin();
    advance(it, 5);
    numbers.insert(it, 10);

    // 2. Efficient Removal
    numbers.remove_if([](int n) { return n % 2 == 0; });
}

// String Best Practices
void stringBestPractices() {
    cout << "\n=== String Best Practices ===\n";

    // 1. String Concatenation
    {
        const int ITERATIONS = 1000;

        // BAD: Using += for many concatenations
        string bad;
        for (int i = 0; i < ITERATIONS; i++) {
            bad += "hello";  // Reallocates frequently
        }

        // BETTER: Using reserve
        string better;
        better.reserve(ITERATIONS * 5);  // "hello" is 5 chars
        for (int i = 0; i < ITERATIONS; i++) {
            better += "hello";
        }

        // BEST: Using ostringstream for complex concatenations
        ostringstream best;
        for (int i = 0; i < ITERATIONS; i++) {
            best << "hello";
        }
        string result = best.str();
    }

    // 2. String View for String Arguments
    {
        // BAD: Taking string by value unnecessarily
        auto processStringBad = [](string str) { return str.length(); };

        // GOOD: Using string_view for read-only string operations
        auto processStringGood = [](string_view sv) { return sv.length(); };
        // std::string_view (introduced in C++17) is a non-owning view into a string.
        // Think of it as a lightweight pointer to an existing string, without the overhead of
        // copying or managing the string's memory.
        // It's incredibly useful for improving performance,
        // especially when dealing with function arguments and read-only string operations.


        string str = "test";
        processStringGood(str);        // Works with string
        processStringGood("literal");  // Works with literal, no allocation
    }

    // 3. Efficient Number to String Conversion
    {
        int num = 12345;

        // BAD: Using string streams for simple conversions
        ostringstream oss;
        oss << num;
        string str1 = oss.str();

        // BETTER: Using to_string for simple cases
        string str2 = to_string(num);

        // BEST: Preallocate when converting many numbers
        vector<int> numbers(1000, 12345);
        string result;
        result.reserve(numbers.size() * 6);  // Each number takes ~5-6 chars
        for (int n : numbers) {
            result += to_string(n) + ",";
        }
    }

    // 4. String Splitting
    {
        string input = "apple,banana,orange";

        // BAD: Using find in a loop
        vector<string> tokensBad;
        size_t pos = 0;
        while ((pos = input.find(',')) != string::npos) {
            tokensBad.push_back(input.substr(0, pos));
            input.erase(0, pos + 1);
        }

        // BETTER: Using stringstream
        vector<string> tokensBetter;
        stringstream ss(input);
        string token;
        while (getline(ss, token, ',')) {
            tokensBetter.push_back(token);
        }
    }

    // 5. String Search and Replace
    {
        string text = "Hello World! Hello Again!";
        string search = "Hello";
        string replace = "Hi";

        // BAD: Manual position tracking
        size_t pos = 0;
        while ((pos = text.find(search, pos)) != string::npos) {
            text.replace(pos, search.length(), replace);
            pos += replace.length();
        }

        // BETTER: Using regex for complex patterns
        // #include <regex>
        // text = regex_replace(text, regex("Hello"), "Hi");
    }
}


int main() {
    stringBestPractices();
    vectorBestPractices();
    mapBestPractices();
    listBestPractices();

    /**
     Additional STL Best Practices
        - Prefer emplace over insert: Construct elements in-place to avoid copies/moves.
        - Use range-based for loops: Cleaner and less error-prone.
        - Use algorithms from <algorithm>: Leverage standard algorithms for efficiency and readability.
        - Understand iterator invalidation: Be aware of operations that invalidate iterators.
        - Use std::optional for optional values.
        - Use std::variant for types that can hold different types.
        - Prefer std::array for fixed-size arrays.
        - Use std::unordered_map and std::unordered_set when order doesn't matter.
        - Be mindful of exception safety.
    */

    return 0;
}
