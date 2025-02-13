


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