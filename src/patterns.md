
## Preprocesor directive

```cpp
#ifndef ERROR_H
#define ERROR_H

#include <string>

class Error {
    // ... Error class definition
};

#endif // ERROR_H
``` 

vs modern 

```cpp
pragma once
```


Helps to tell pre-procesor to only incldue a header file once (In case multiple files are referencing the same header file via `#include`)

## Common practice to name alias

- `using` is modern
- You might also see `typedef <> <>` in places

```
using RefCountedSession = std::shared_ptr<Session>;
using RefCountedCache = std::shared_ptr<Cache>;
using RefCountedWorker = std::shared_ptr<Worker>;

// Unique ownership
using DatabasePtr = std::unique_ptr<Database>;
using LoggerPtr = std::unique_ptr<Logger>;

// Non-owning references
using EngineRef = Engine&;
using StorageRef = Storage&;

// Example usage
class System {
    DatabasePtr db_;
    LoggerPtr log_;
    RefCountedSession session_;
public:
    System() 
        : db_(std::make_unique<Database>())
        , log_(std::make_unique<Logger>())
        , session_(std::make_shared<Session>()) 
    {}
    
    void addWorker() {
        RefCountedWorker worker = std::make_shared<Worker>();
        workers_.push_back(worker);
    }
private:
    std::vector<RefCountedWorker> workers_;
};
```

## Callback functions

```cpp
class Error {} // sample Error class with various categories and all.

// std::function<return_type(Args...)>;
using on_operation_complete = std::function<void(const Error&)>;

```

## Function Binding

Transforms complex function signatures into simpler ones by binding/capturing
known parameters, leaving only the "future" parameters unbound.


```cpp
template<typename T>
class Service {
    // Original complex function
    void complexOperation(
        Database* db,      // Known at bind time
        Config* cfg,       // Known at bind time
        UserContext* ctx,  // Known at bind time
        std::vector<T> items,    // Known at bind time
        CallbackResult result    // Will get this later
    );

    void example() {
        // We have these values now
        auto* db = getCurrentDb();
        auto* cfg = getConfig();
        auto* ctx = getUserContext();
        auto items = getItems();

        // Modern approach (preferred) - using lambda
        auto handler = [this, db, cfg, ctx, items]
            (CallbackResult result) {
            complexOperation(db, cfg, ctx, items, result);
        };

        // handler is now: void(CallbackResult)
        
        // Legacy approach - using std::bind
        auto handler_old = std::bind(
            &Service::complexOperation,
            this,
            db,
            cfg, 
            ctx,
            items,
            std::placeholders::_1 // Only this parameter remains "open"
        );
         // handler_old is now: void(CallbackResult)

        // Both transform: void(DB*, Cfg*, Ctx*, vector<T>, Result)
        //          into: void(Result)

        // Usage:
        someAsyncOperation(handler);
    }
};
```

## enable_shared_from_this

ref - https://stackoverflow.com/a/5548314


Bad - 
```
struct S
{
  shared_ptr<S> dangerous()
  {
     return shared_ptr<S>(this);   // don't do this!
  }
};

int main()
{
   shared_ptr<S> sp1(new S);
   shared_ptr<S> sp2 = sp1->dangerous();
   return 0;
}
```

Good - 

```
struct S : enable_shared_from_this<S>
{
  shared_ptr<S> not_dangerous()
  {
    return shared_from_this();
  }
};

int main()
{
   shared_ptr<S> sp1(new S);
   shared_ptr<S> sp2 = sp1->not_dangerous();
   return 0;
}
```

## Practical uses of enable_shared_from_this

Common usage of `enable_shared_from_this` 

1. The object needs to register itself somewhere else
2. That registration needs to outlive the current function call
3. We need guarantees about the object's lifetime during callbacks or delayed operations


Lets consider async operation example implemented in different ways to highlight why `enable_shared_from_this` is the best solution.

1. Using `enable_shared_from_this` (safe approach):
```cpp
class NetworkHandler : public enable_shared_from_this<NetworkHandler> {
    void initiateAsyncOperation() {
        asyncOperation([self = shared_from_this()](Result r) {
            self->handleResult(r);
        });
    }

    void handleResult(Result r) {
        // Process result
    }
};

// Usage
auto handler = make_shared<NetworkHandler>();
handler->initiateAsyncOperation();
// Even if 'handler' is destroyed, the callback is safe
```

2. Passing Original shared_ptr (cumbersome approach):
```cpp
class NetworkHandler {
    void initiateAsyncOperation(shared_ptr<NetworkHandler> originalPtr) {
        asyncOperation([originalPtr](Result r) {
            originalPtr->handleResult(r);
        });
    }
};

// Usage - must remember to pass shared_ptr
auto handler = make_shared<NetworkHandler>();
handler->initiateAsyncOperation(handler); // Awkward!
```

3. Using Raw Pointer (dangerous approach):
```cpp
class NetworkHandler {
    void initiateAsyncOperation() {
        asyncOperation([this](Result r) {  // DANGEROUS!
            this->handleResult(r);
        });
    }
};

// Usage
auto handler = make_shared<NetworkHandler>();
handler->initiateAsyncOperation();
// If 'handler' is destroyed before callback executes,
// we'll have a dangling pointer!
```

4. Creating Independent shared_ptr (incorrect approach):
```cpp
class NetworkHandler {
    void initiateAsyncOperation() {
        asyncOperation([self = shared_ptr<NetworkHandler>(this)](Result r) {  // WRONG!
            self->handleResult(r);
        });
    }
};

// Usage
auto handler = make_shared<NetworkHandler>();
handler->initiateAsyncOperation();
// Will cause double deletion!
```

Here's a more complex real-world example showing why `enable_shared_from_this` is valuable:

```cpp
class DownloadManager : public enable_shared_from_this<DownloadManager> {
    queue<string> downloadQueue;
    bool isProcessing = false;
    
    void startProcessing() {
        if (isProcessing || downloadQueue.empty()) return;
        
        isProcessing = true;
        auto url = downloadQueue.front();
        downloadQueue.pop();

        // Capture shared_from_this() in multiple callbacks
        auto self = shared_from_this();
        
        httpClient.asyncGet(url,
            // Success callback
            [self](Response resp) {     // each capture of self will automatically increment the ref count
                self->handleSuccess(resp);
                self->isProcessing = false;
                // Continue processing queue
                self->startProcessing();
            },
            // Error callback
            [self](Error err) {
                self->handleError(err);
                self->isProcessing = false;
                // Retry or continue
                self->startProcessing();
            },
            // Progress callback
            [self](int progress) {
                self->updateProgress(progress);
            }
        );
    }
};
```

The benefits of `enable_shared_from_this` here are:
1. Each callback has a guaranteed-valid pointer to the DownloadManager
2. The DownloadManager won't be destroyed until all callbacks complete
3. We don't need to manually track or pass shared_ptrs
4. Multiple callbacks can safely hold references to the same object

Without `enable_shared_from_this`, managing these cascading async operations while ensuring proper lifetime management would be much more complex and error-prone.

## Casting

## `decltype` 

`decltype(comp)` is a type deduction operator that gives you the exact type of the expression `comp`. It's particularly useful with lambdas because each lambda has its own unique, compiler-generated type that you can't write directly.

Let's break it down:

```cpp
// Let's define a lambda
auto comp = [](int a, int b) { return a > b; };

// decltype(comp) gives us the actual type of this lambda
// It might look something like this internally (simplified):
// class __lambda_8472947 {
//     bool operator()(int a, int b) const { return a > b; }
// };
```

Some concrete examples:

```cpp
// Simple types
int x = 42;
decltype(x) y = 10;  // y is an int

std::string str = "hello";
decltype(str) another_str = "world";  // another_str is a std::string

// With lambdas
auto lambda1 = [](int x) { return x * 2; };
decltype(lambda1) lambda2 = lambda1;  // Creates another lambda of the same type

// Real-world usage with priority queue
auto comp = [](int a, int b) { return a > b; };
std::priority_queue<int, std::vector<int>, decltype(comp)> pq(comp);

// What's actually happening above:
// 1. decltype(comp) gets the unique type of our lambda
// 2. This type becomes the third template parameter of priority_queue
// 3. The constructor gets the actual comparator object
```

To really understand why we need this, let's see what happens without `decltype`:

```cpp
auto comp = [](int a, int b) { return a > b; };

// This won't compile - the compiler doesn't know what type to use
// std::priority_queue<int, std::vector<int>, comp> pq;  // ERROR!

// These are different types!
auto comp1 = [](int a, int b) { return a > b; };
auto comp2 = [](int a, int b) { return a > b; };

// This will print "false" - they're different types
std::cout << std::is_same_v<decltype(comp1), decltype(comp2)> << "\n";

// But decltype lets us specify the exact type we want
using Comp1Type = decltype(comp1);
using Comp2Type = decltype(comp2);

std::priority_queue<int, std::vector<int>, Comp1Type> pq1(comp1);  // Works!
// std::priority_queue<int, std::vector<int>, Comp2Type> pq2(comp1);  // ERROR!
```

You can also see the type (in a way) using type_info:
```cpp
#include <typeinfo>
std::cout << typeid(comp).name() << "\n";  // Will print something cryptic
```

The main points about `decltype`:
1. It gives you the exact type of an expression
2. It's evaluated at compile time
3. It's essential when working with lambda types
4. It's useful for template metaprogramming
5. It helps maintain type safety while working with complex types

Without `decltype`, it would be very difficult to use lambdas as template parameters because we wouldn't have a way to specify their types.