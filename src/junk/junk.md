### About

Just some scratch space for temporary notes

### Scratch space

```cpp
auto comp = [](pair<int,int>& a, pair<int,int>& b) {
    return a.first > b.first;  // min heap based on frequency
};
```


```cpp
priority_queue<pair<int,int>, 
                      vector<pair<int,int>>, 
                      decltype(comp)> pq(comp);
```


```cpp

string_view str = "Hello World";
size_t pos = str.find("xyz");  
if (pos != string_view::npos) {
    // substring found
} else {
    // substring not found
}
```