## Concurrency Design Patterns

### Thread Pool

```cpp
class ThreadPool {
private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex queue_mutex;
    std::condition_variable condition;
    bool stop;

public:
    ThreadPool(size_t threads) : stop(false) {
        for(size_t i = 0; i < threads; ++i) {
            workers.emplace_back([this] {
                while(true) {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(queue_mutex);
                        condition.wait(lock, [this]{ 
                            return stop || !tasks.empty(); 
                        });
                        
                        if(stop && tasks.empty()) return;
                        
                        task = std::move(tasks.front());
                        tasks.pop();
                    }
                    task();
                }
            });
        }
    }

    template<class F>
    void enqueue(F&& f) {
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            tasks.emplace(std::forward<F>(f));
        }
        condition.notify_one();
    }

    ~ThreadPool() {
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            stop = true;
        }
        condition.notify_all();
        for(std::thread &worker: workers) {
            worker.join();
        }
    }
};
```

### Monitor Pattern

```cpp
class ThreadSafeQueue {
private:
    std::mutex m;
    std::condition_variable cv;
    std::queue<int> data;
public:
    void push(int value) {
        std::lock_guard<std::mutex> lock(m);
        data.push(value);
        cv.notify_one();
    }
    
    bool pop(int& value) {
        std::unique_lock<std::mutex> lock(m);
        cv.wait(lock, [this]{ return !data.empty(); });
        value = data.front();
        data.pop();
        return true;
    }
    
    bool try_pop(int& value) {
        std::lock_guard<std::mutex> lock(m);
        if (data.empty()) return false;
        value = data.front();
        data.pop();
        return true;
    }
};
```

### Active Object Pattern

```cpp
class ActiveObject {
private:
    ThreadPool pool;
    std::atomic<bool> running{true};
    
public:
    ActiveObject() : pool(1) {}  // Single-threaded executor
    
    template<typename F, typename... Args>
    auto enqueue(F&& f, Args&&... args) 
        -> std::future<typename std::invoke_result<F, Args...>::type> {
        
        using return_type = typename std::invoke_result<F, Args...>::type;
        
        auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );
            
        std::future<return_type> result = task->get_future();
        
        pool.enqueue([task](){ (*task)(); });
        
        return result;
    }
    
    void shutdown() {
        running = false;
    }
};

// Usage
ActiveObject calculator;
auto result = calculator.enqueue([](int a, int b) { return a + b; }, 2, 3);
std::cout << "Result: " << result.get() << std::endl;
```

### Lock-Free Queue

```cpp
template<typename T>
class LockFreeQueue {
private:
    struct Node {
        std::shared_ptr<T> data;
        std::atomic<Node*> next;
        
        Node() : next(nullptr) {}
    };
    
    std::atomic<Node*> head;
    std::atomic<Node*> tail;
    
public:
    LockFreeQueue() {
        Node* dummy = new Node;
        head.store(dummy);
        tail.store(dummy);
    }
    
    void push(T value) {
        std::shared_ptr<T> new_data = std::make_shared<T>(std::move(value));
        Node* new_node = new Node;
        new_node->data = new_data;
        
        Node* old_tail = tail.load();
        while (!old_tail->next.compare_exchange_weak(
            nullptr, new_node, std::memory_order_release, std::memory_order_relaxed)) {
            old_tail = tail.load();
        }
        
        // Try to update tail
        tail.compare_exchange_strong(old_tail, new_node);
    }
    
    std::shared_ptr<T> pop() {
        Node* old_head = head.load();
        Node* next;
        
        do {
            next = old_head->next.load();
            if (!next) return nullptr; // Queue empty
        } while (!head.compare_exchange_weak(
            old_head, next, std::memory_order_release, std::memory_order_relaxed));
            
        std::shared_ptr<T> result = next->data;
        delete old_head;
        return result;
    }
};
```

### Producer-Consumer Pattern

```cpp
template<typename T>
class BoundedQueue {
private:
    std::queue<T> buffer;
    size_t capacity;
    std::mutex mtx;
    std::condition_variable not_full;
    std::condition_variable not_empty;
    
public:
    BoundedQueue(size_t capacity) : capacity(capacity) {}
    
    void produce(T item) {
        std::unique_lock<std::mutex> lock(mtx);
        not_full.wait(lock, [this]{ return buffer.size() < capacity; });
        
        buffer.push(std::move(item));
        
        lock.unlock();
        not_empty.notify_one();
    }
    
    T consume() {
        std::unique_lock<std::mutex> lock(mtx);
        not_empty.wait(lock, [this]{ return !buffer.empty(); });
        
        T item = std::move(buffer.front());
        buffer.pop();
        
        lock.unlock();
        not_full.notify_one();
        
        return item;
    }
};
```
