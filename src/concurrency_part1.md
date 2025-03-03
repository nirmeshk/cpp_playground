# Comprehensive Guide to C++ Concurrency & Synchronization

## Table of Contents
- [Introduction to C++ Concurrency](#introduction-to-c-concurrency)
- [Threads in C++](#threads-in-c)
- [Mutexes & Locks](#mutexes--locks)
- [The C++ Memory Model & Memory Ordering](#the-c-memory-model--memory-ordering)
  - [Memory Orders](#memory-orders)
  - [Memory Barriers & Fences](#memory-barriers--fences)
  - [Synchronization Patterns](#synchronization-patterns)
- [Atomics](#atomics)
  - [Basic Atomic Operations](#basic-atomic-operations)
  - [Atomic vs Mutexes](#atomic-vs-mutexes)
  - [Performance Considerations](#atomic-performance-considerations)
- [Synchronization Tools](#synchronization-tools)
  - [Condition Variables](#condition-variables)
  - [Futures & Promises](#futures--promises)
  - [One-time Initialization](#one-time-initialization)
  - [Barriers & Latches (C++20)](#barriers--latches-c20)
- [Performance Considerations](#performance-considerations)
- [Concurrency Design Patterns](concurrency_part2.md) -- Continue to Part 2

## Introduction to C++ Concurrency

C++11 introduced a standardized threading model with built-in support for multi-threaded programming. This was a significant improvement over the previous need for platform-specific libraries like pthreads or Windows threads.

The C++ concurrency library provides:
- Thread management
- Synchronization primitives
- Atomic operations
- A well-defined memory model

## Threads in C++

```cpp
#include <thread>

void task(int x, const std::string& str) {
    // Thread function body
}

int main() {
    // Create and start a thread
    std::thread t1(task, 42, "hello");
    
    // Lambda with capture
    auto lambda = [](int x) { /* work */ };
    std::thread t2(lambda, 10);
    
    // Check if joinable
    if(t1.joinable()) {
        t1.join(); // Wait for completion (must call join or detach)
    }
    
    // thread::hardware_concurrency() gives CPU cores count
    unsigned int cores = std::thread::hardware_concurrency();
    
    return 0;
} // Not joining t2 here will crash (std::terminate)
```

**Key Points:**
- Always join/detach before destruction or you'll get `std::terminate`
- Thread ID available via `t.get_id()`
- Can't copy threads, only move them
- C++20 adds `std::jthread` with automatic joining on destruction

## Mutexes & Locks

Mutexes (mutual exclusion) are the primary mechanism for protecting shared data from concurrent access:

```cpp
#include <mutex>
#include <shared_mutex> // C++14

std::mutex m;
std::shared_mutex sm; // Allows multiple readers, exclusive writers

// RAII Locks:
{
    // Exclusive lock (most common)
    std::lock_guard<std::mutex> lock(m);
    // Critical section...
} // Automatically unlocks when out of scope

// More flexible lock with manual unlock capability
std::unique_lock<std::mutex> ulock(m, std::defer_lock); // Don't lock immediately
ulock.lock();
// ...
ulock.unlock(); // Can unlock early
// ...
ulock.lock(); // Can relock

// Read-only lock (C++14)
{
    std::shared_lock<std::shared_mutex> rlock(sm); // Multiple can exist
    // Read-only operations...
}

// Write lock
{
    std::unique_lock<std::shared_mutex> wlock(sm); // Exclusive access
    // Write operations...
}

// Deadlock prevention when locking multiple mutexes
{
    std::unique_lock<std::mutex> lock1(m1, std::defer_lock);
    std::unique_lock<std::mutex> lock2(m2, std::defer_lock);
    std::lock(lock1, lock2); // Locks both without deadlock risk
    // Critical section with both resources...
}

// C++17 scoped_lock (simpler syntax for above)
{
    std::scoped_lock lock(m1, m2, m3); // Locks all atomically without deadlock
    // Critical section...
}
```

### `std::lock` and `std::unique_lock` Deep Dive

`std::unique_lock` provides flexible locking semantics:

```cpp
void basic_usage() {
    // Immediate locking (like lock_guard)
    std::unique_lock<std::mutex> lock1(m);
    
    // Deferred locking (don't lock yet)
    std::unique_lock<std::mutex> lock2(m, std::defer_lock);
    lock2.lock(); // Manual lock later
    
    // Try locking (non-blocking attempt)
    std::unique_lock<std::mutex> lock3(m, std::try_to_lock);
    if (lock3.owns_lock()) {
        // Lock was acquired successfully
    }
    
    // Lock with timeout (requires timed_mutex)
    std::timed_mutex tm;
    std::unique_lock<std::timed_mutex> lock4(tm, std::defer_lock);
    if (lock4.try_lock_for(std::chrono::milliseconds(100))) {
        // Got the lock within 100ms
    }
}
```

**Key Features of `std::unique_lock`:**

1. **Manual lock/unlock capability**: Unlike `lock_guard`, you can unlock early and relock
   ```cpp
   void process_data() {
       std::unique_lock<std::mutex> lock(data_mutex);
       
       // Make a local copy of protected data
       auto data_copy = shared_data;
       
       // Unlock before expensive operation
       lock.unlock();
       
       // Do expensive work without holding lock
       auto result = process(data_copy);
       
       // Relock to update shared data
       lock.lock();
       shared_data = result;
   }
   ```

2. **Movable (transferable) ownership**: You can pass lock ownership to functions
   ```cpp
   void process_with_lock(std::unique_lock<std::mutex> lock) {
       // Lock already owned, continue with critical section
   }
   
   void caller() {
       std::unique_lock<std::mutex> lock(m);
       process_with_lock(std::move(lock)); // Transfer ownership
       // lock no longer owns the mutex here
   }
   ```

3. **State interrogation**: Check if lock is currently held
   ```cpp
   if (lock.owns_lock()) {
       // We have the lock
   }
   ```

**Understanding `std::lock`:**

`std::lock` is a function that atomically locks multiple mutexes without risk of deadlock:

```cpp
void transfer(Account& from, Account& to, double amount) {
    // Create locks but don't lock yet
    std::unique_lock<std::mutex> lock_from(from.mutex, std::defer_lock);
    std::unique_lock<std::mutex> lock_to(to.mutex, std::defer_lock);
    
    // Lock both mutexes without deadlock risk
    std::lock(lock_from, lock_to);
    
    // Critical section
    from.balance -= amount;
    to.balance += amount;
}
```

**How `std::lock` Works:**

1. **Deadlock Prevention Algorithm**: Uses a variant of the lock hierarchy algorithm
2. **All-or-Nothing Guarantee**: Either locks all mutexes or none
3. **Roll-forward Approach**: If it can't lock all mutexes, it will keep trying
4. **Works with Any Lockable Types**: Compatible with any type providing lock/unlock methods

### Rules for Correct Mutex Usage

1. **Protect data, not code**:
   ```cpp
   class Account {
       mutable std::mutex m;
       double balance = 0.0;
   public:
       void deposit(double amount) {
           std::lock_guard<std::mutex> lock(m);
           balance += amount;
       }
       
       // Getters need protection too!
       double getBalance() const {
           std::lock_guard<std::mutex> lock(m);
           return balance;
       }
   };
   ```

2. **Always protect reads AND writes**: Without mutex on getters, you can read partially-written data.

3. **Use the same mutex for related data**: All fields that are logically part of one "thing" should share a mutex.

4. **Lock duration principle**: Hold locks for the minimum time necessary, but ensure complete operations are atomic.

5. **Make members private**: Public fields cannot be properly protected.

6. **Monitor pattern**: One mutex per class that guards all data. The monitor pattern is a fundamental concurrency design pattern where all class data is protected by a single mutex. It provides a clean, object-oriented approach to thread synchronization.


7. **Consider thread safety in your API design**: Provide operations that maintain invariants atomically.
   ```cpp
   // Bad API: Requires external synchronization
   int front();
   void pop();
   
   // Good API: Atomic operation
   bool pop(int& value);
   ```

8. **Use RAII for lock management**: Always use `std::lock_guard`, `std::unique_lock`, etc. - never raw `.lock()/.unlock()`.

9. **Mark mutex as mutable for const methods**: If a getter needs a lock, the mutex must be mutable.

10. **Be consistent with lock order**: Always acquire multiple locks in the same order to prevent deadlocks.

11. **Don't return references/pointers to protected data**: They bypass the mutex after return.
    ```cpp
    // DANGEROUS - allows external access to protected data
    std::vector<int>& getItems() {
        std::lock_guard<std::mutex> lock(m);
        return items; // BAD! Internal data exposed without mutex
    }
    
    // SAFE - returns a copy
    std::vector<int> getItemsCopy() {
        std::lock_guard<std::mutex> lock(m);
        return items; // Returns a copy, safe!
    }
    ```

### Mutex Implementation & Performance

Despite using atomics internally, mutexes are slower because:

1. **Multiple Atomic Operations per Mutex Operation**:
   ```cpp
   void mutex::lock() {
       // First atomic: Fast-path attempt
       if (atomic_flag.test_and_set(acquire) == false)
           return; // Got it immediately!
           
       // Second+ atomics: Contention path
       while (true) {
           // Spin for a bit with more atomic operations
           for (int i = 0; i < SPIN_COUNT; i++) {
               if (!atomic_flag.test(relaxed) && 
                   !atomic_flag.test_and_set(acquire))
                   return;
               _mm_pause(); // CPU hint to reduce power in spin-wait
           }
           
           // Failed after spinning, go to sleep (more atomics inside)
           wait_on_address(&atomic_flag, ...);
       }
   }
   ```

2. **System Call Overhead for Contended Locks**:
   ```cpp
   // Linux implementation (simplified)
   void wait_on_address(void* addr, ...) {
       // This involves switching to kernel mode (expensive!)
       syscall(SYS_futex, addr, FUTEX_WAIT, ...);
   }
   ```

3. **The Performance Numbers**:

| Operation | Uncontended Cost | Contended Cost |
|-----------|-----------------|----------------|
| Single atomic RMW | 20-50 cycles | 100-200 cycles |
| Mutex lock+unlock | 100-200 cycles | 5,000-15,000+ cycles |

## The C++ Memory Model & Memory Ordering

The C++ memory model defines how memory operations (reads/writes) become visible to different threads and specifies the ordering constraints between operations.

### Memory Orders

```cpp
std::atomic<int> counter{0};

// Six memory orderings:
counter.store(1, std::memory_order_relaxed);  // Weakest, no synchronization
counter.store(1, std::memory_order_release);  // Makes writes visible to acquiring operations
counter.load(std::memory_order_acquire);      // Sees writes from releasing operations
counter.load(std::memory_order_consume);      // Like acquire but limited to dependent operations (rarely used)
counter.store(1, std::memory_order_acq_rel);  // Combined acquire+release (for read-modify-write)
counter.store(1, std::memory_order_seq_cst);  // Strongest, sequential consistency (default)
```

#### Why Memory Ordering Matters

Without proper ordering, the compiler and CPU can reorder operations:

```cpp
// Thread 1                    // Thread 2
x.store(1, relaxed);           while (!y.load(relaxed));
y.store(1, relaxed);           assert(x.load(relaxed) == 1); // May fail! Cause might get reordered
```

#### Happens-Before Relationships

Memory ordering creates "happens-before" relationships between operations in different threads:

```cpp
// Thread 1                                 // Thread 2
x = 1;                                      if (flag.load(std::memory_order_acquire)) {
flag.store(true,                              // x is guaranteed to be 1 here
    std::memory_order_release);               assert(x.load(relaxed) == 1); 
                                            }
```

Key concepts:
- **Release**: All memory operations before a release are visible to a thread that acquires the same atomic
- **Acquire**: All memory operations after an acquire will see memory operations before the corresponding release
- **Synchronized-with**: An acquire that reads a value from a release forms a synchronization point

#### Sequential Consistency


```cpp
std::atomic<bool> x{false}, y{false};
std::atomic<int> z{0};

// Thread 1                     // Thread 2
x.store(true);                  y.store(true);
if (y.load()) {                 if (x.load()) {
    z.store(1);                     z.store(2);
}                               }
```

With sequential consistency, `z` will be either 0, 1, or 2 - never any other value. All threads see operations in a global consistent order.

**Release-Acquire Synchronization**:

```cpp
std::atomic<bool> ready{false};
int data = 0;

// Thread 1
data = 42;
ready.store(true, std::memory_order_release);

// Thread 2
while (!ready.load(std::memory_order_acquire));
assert(data == 42); // Always true
```

This pattern guarantees that when Thread 2 sees `ready == true`, it will also see all memory operations (including non-atomic ones) that happened before the release in Thread 1.

**Relaxed Atomics (weakest)**:

```cpp
std::atomic<int> counter{0};

// Can be called from multiple threads
void increment() {
    counter.fetch_add(1, std::memory_order_relaxed);
}
```

Relaxed operations only guarantee atomicity - no synchronization. Use when:
- Only the atomic variable itself matters
- You don't care about the ordering of other memory operations
- Common for counters, statistics, flags without associated data

**Memory Ordering Performance Costs**:

| Memory Order | Performance Impact | x86/x64 | ARM/POWER | When to use |
|--------------|-------------------|---------|-----------|-------------|
| Relaxed | Minimal (0-5%) | Almost free | Simple instruction | Counters, statistics |
| Acquire/Release | Moderate (5-20%) | Compiler barriers | LDAR/STLR instructions | Data publishing |
| Sequential Consistency | Significant (20-50%) | MFENCE | Full barriers | When correctness > performance |

### Memory Barriers & Fences

Memory fences enforce ordering without being attached to a specific atomic operation:

```cpp
std::atomic<bool> flag1{false}, flag2{false};
int data = 0;

// Thread 1
data = 42;
std::atomic_thread_fence(std::memory_order_release); // Fence instead of release operation
flag1.store(true, std::memory_order_relaxed);

// Thread 2
while (!flag1.load(std::memory_order_relaxed));
std::atomic_thread_fence(std::memory_order_acquire); // Fence instead of acquire operation
assert(data == 42); // Always true
```

#### Types of Memory Fences

1. **Acquire Fence**:
   ```cpp
   std::atomic_thread_fence(std::memory_order_acquire);
   ```
   - Creates an acquire barrier
   - All memory operations after the fence happen-after prior releases
   - Prevents reordering of loads from before the fence to after it

2. **Release Fence**:
   ```cpp
   std::atomic_thread_fence(std::memory_order_release);
   ```
   - Creates a release barrier
   - All memory operations before the fence happen-before subsequent acquires
   - Prevents reordering of stores from after the fence to before it

3. **Acquire-Release Fence**:
   ```cpp
   std::atomic_thread_fence(std::memory_order_acq_rel);
   ```
   - Combines both acquire and release semantics
   - Prevents reordering across the fence in both directions
   - Useful for read-modify-write operations

4. **Sequential Consistency Fence**:
   ```cpp
   std::atomic_thread_fence(std::memory_order_seq_cst);
   ```
   - Strongest fence, full memory barrier
   - Prevents all reordering across the fence
   - Establishes a total order with all other seq_cst operations

#### Hardware Implementation of Memory Barriers

Different CPU architectures implement memory barriers differently:

**x86/x64**:
- Already has strong ordering - most loads and stores act as acquire/release
- Only needs barriers for StoreLoad ordering (full fence)
- `MFENCE` instruction for full barriers
- `SFENCE` (store fence) and `LFENCE` (load fence) for partial barriers

**ARM/PowerPC**:
- Weaker hardware model - needs explicit barriers
- `DMB` (Data Memory Barrier) on ARM
- Various types: full system, inner/outer shareable
- `SYNC`, `LWSYNC` (lightweight sync) on POWER

#### When to Use Fences vs. Atomic Operations

Fences are useful when:
- You need to synchronize multiple atomic variables together
- You want to create a synchronization point without modifying an atomic variable
- Working with externally synchronized memory (e.g., hardware registers)
- Optimizing by grouping multiple operations under one fence

```cpp
// Instead of:
a.store(1, std::memory_order_release);
b.store(2, std::memory_order_release);
c.store(3, std::memory_order_release);

// You can use:
a.store(1, std::memory_order_relaxed);
b.store(2, std::memory_order_relaxed);
c.store(3, std::memory_order_relaxed);
std::atomic_thread_fence(std::memory_order_release);
```

#### Performance Considerations for Fences

- Full barriers (seq_cst): ~100-150 cycles on x86, ~200-400 on ARM
- Acquire/release fences: ~50-100 cycles
- Flush store buffers (makes writes immediately visible)
- Invalidate load caches (forces fresh reads)
- Block CPU reordering (reduces instruction-level parallelism)
- Generate cache coherence traffic (slows down all cores)

### Synchronization Patterns

#### 1. Double-Checked Locking Pattern

```cpp
std::atomic<Singleton*> instance{nullptr};
std::mutex init_mutex;

Singleton* getInstance() {
    Singleton* p = instance.load(std::memory_order_acquire);
    if (!p) {
        std::lock_guard<std::mutex> lock(init_mutex);
        p = instance.load(std::memory_order_relaxed);
        if (!p) {
            p = new Singleton();
            instance.store(p, std::memory_order_release);
        }
    }
    return p;
}
```

#### 2. Publisher-Subscriber Pattern

```cpp
struct Message {
    int id;
    std::string data;
};

std::atomic<Message*> message{nullptr};

// Publisher
void publish(int id, const std::string& data) {
    Message* m = new Message{id, data};
    message.store(m, std::memory_order_release);
}

// Subscriber
void process() {
    Message* m = message.load(std::memory_order_acquire);
    if (m) {
        // Process message safely - all fields will be visible
        use(m->id, m->data);
    }
}
```

#### 3. Read-Copy-Update (RCU) Pattern

```cpp
struct Data {
    std::string content;
    // other fields...
};

std::atomic<Data*> current_data{new Data{"initial"}};

// Writer
void update(const std::string& new_content) {
    // Create new version (doesn't disturb readers)
    Data* new_data = new Data{new_content};
    
    // Publish new version with release semantics
    Data* old = current_data.exchange(new_data, std::memory_order_acq_rel);
    
    // Cleanup old version when safe (in practice, would delay this)
    delete old;
}

// Reader
std::string read() {
    // Get current snapshot with acquire semantics
    Data* data = current_data.load(std::memory_order_acquire);
    std::string result = data->content;
    return result;
}
```

#### 4. Release Sequences

```cpp
std::atomic<int> counter{0};

// Thread 1
counter.store(1, std::memory_order_release);

// Thread 2
counter.fetch_add(1, std::memory_order_relaxed); // Makes counter = 2

// Thread 3
if (counter.load(std::memory_order_acquire) == 2) {
    // Can see Thread 1's writes even though Thread 2 used relaxed
}
```

A release sequence allows an acquire operation to synchronize with an earlier release, even if there are intervening atomic operations with relaxed ordering.

## Atomics

Atomic operations provide thread-safe access to single variables without locks.

### Basic Atomic Operations

```cpp
#include <atomic>

// Basic usage
std::atomic<int> counter{0};
counter++; // Atomic increment
int value = counter.load(); // Atomic load

// Compare-and-swap operations
int expected = 5;
bool exchanged = counter.compare_exchange_strong(
    expected, 10, 
    std::memory_order_acq_rel
);
// If counter was 5, it's now 10 and exchanged is true
// If counter wasn't 5, expected now contains the actual value

// Atomic operations on regular data
// (No need for std::atomic wrapper)
int regular_var = 0;
int old_value = std::atomic_fetch_add(&regular_var, 1); // C++20

// Lock-free check
bool is_lock_free = counter.is_lock_free();

// Other atomic types
std::atomic<bool> flag{false};
std::atomic<void*> ptr{nullptr};
std::atomic_flag lock = ATOMIC_FLAG_INIT; // Simplest atomic, always lock-free
```

### Atomic vs Mutexes

| Aspect | Atomics | Mutexes |
|--------|---------|---------|
| **Operation Scope** | Single variable | Any code block |
| **Blocking** | Non-blocking (lock-free) | Can block thread |
| **Performance** | Faster (20-50 cycles) | Slower (100-15000+ cycles) |
| **Memory Impact** | Direct hardware instructions | OS support for waiting |
| **Composability** | Poor (individual operations) | Good (critical sections) |
| **Use Case** | Simple counters, flags, pointers | Complex data structures |

### Atomic Performance Considerations

1. **Hardware Support**:
   - Some atomic operations map directly to CPU instructions
   - Others may require locks on some platforms if not natively supported
   - `is_lock_free()` checks if hardware supports lock-free implementation

2. **CAS Loop Contention**:
   ```cpp
   // High contention pattern, exponential slowdown with thread count
   while (!value.compare_exchange_weak(expected, new_value)) {
       expected = old_value;
   }

   // Better pattern with backoff
   int backoff = 1;
   while (!value.compare_exchange_weak(expected, new_value)) {
       expected = old_value;
       if (backoff > 1) std::this_thread::sleep_for(std::chrono::nanoseconds(backoff));
       backoff = std::min(backoff * 2, 1000); // Exponential backoff
   }
   ```

3. **False Sharing**:
   ```cpp
   // BAD: Two counters likely on same cache line
   std::atomic<int> counter1;
   std::atomic<int> counter2;

   // BETTER: Force different cache lines
   alignas(64) std::atomic<int> counter1;
   alignas(64) std::atomic<int> counter2;
   ```

4. **Read-Heavy Workloads**:
   For read-mostly structures, consider lock-free read + mutex-protected write:
   ```cpp
   std::atomic<Data*> current_data;
   std::mutex write_mutex;
   
   // Reader - lock-free
   Data* read() {
       return current_data.load(std::memory_order_acquire);
   }
   
   // Writer - uses mutex
   void update(const Data& new_data) {
       std::lock_guard<std::mutex> lock(write_mutex);
       Data* new_copy = new Data(new_data);
       Data* old = current_data.exchange(new_copy, std::memory_order_acq_rel);
       // Schedule old for deletion
   }
   ```

## Synchronization Tools

### Condition Variables

Condition variables allow threads to wait for a specific condition to become true:

```cpp
#include <condition_variable>
#include <mutex>
#include <queue>

std::mutex m;
std::condition_variable cv;
std::queue<int> work_queue;
bool done = false;

// Producer
void producer() {
    for(int i = 0; i < 10; i++) {
        {
            std::lock_guard<std::mutex> lock(m);
            work_queue.push(i);
        }
        cv.notify_one(); // Wake one waiting thread
        // Or cv.notify_all() to wake all waiters
    }
    
    {
        std::lock_guard<std::mutex> lock(m);
        done = true;
    }
    cv.notify_all();
}

// Consumer
void consumer() {
    while(true) {
        std::unique_lock<std::mutex> lock(m);
        // Wait until queue has data or we're done
        cv.wait(lock, []{
            return !work_queue.empty() || done;
        });
        
        if(done && work_queue.empty()) break;
        
        // Process work
        int data = work_queue.front();
        work_queue.pop();
        lock.unlock(); // Unlock before processing
        
        process(data);
    }
}
```

#### Internal Working Mechanism

1. **Lock Handoff**:
   ```cpp
   cv.wait(lock, predicate);
   ```
   - Atomically releases the lock and puts thread to sleep
   - When notified, reacquires the lock before checking the predicate
   - If predicate is false, releases lock and sleeps again
   - If predicate is true, returns with lock held

2. **System Implementation**:
   - Uses OS-specific primitives:
     - Linux: futex system call
     - Windows: WaitOnAddress/WakeByAddress
   - Thread is removed from CPU scheduling until notification

3. **Spurious Wakeups**:
   - Threads may wake without notification (hardware/OS reasons)
   - Always use a predicate function to recheck condition
   - Never do: `cv.wait(lock); // WRONG! No predicate check`

#### Advanced CV Techniques

1. **Timed Waits**:
   ```cpp
   // Wait up to 100ms
   if (cv.wait_for(lock, 100ms, [&]{ return !task_queue.empty(); })) {
       // Condition became true within timeout
       process(task_queue.front());
       task_queue.pop();
   } else {
       // Timeout occurred
       handle_timeout();
   }
   ```

2. **Multiple Producers Pattern**:
   ```cpp
   // Producer
   {
       std::lock_guard<std::mutex> lock(mtx);
       task_queue.push(task);
       is_data_ready = true;
   }
   cv.notify_all(); // Wake all consumers to race for this task
   ```

3. **Multiple Condition Variables**:
   ```cpp
   std::condition_variable data_cv;  // For data availability
   std::condition_variable space_cv; // For buffer space availability
   
   // Producer
   {
       std::unique_lock<std::mutex> lock(mtx);
       space_cv.wait(lock, [&]{ return buffer.size() < MAX_SIZE; });
       buffer.push(item);
       lock.unlock();
       data_cv.notify_one(); // Notify consumer about new data
   }
   ```

#### Condition Variables vs. Other Synchronization

| Aspect | Condition Variables | Mutexes | Atomics | Futures/Promises | Barriers |
|--------|---------------------|---------|---------|------------------|----------|
| **Primary Purpose** | Wait for conditions | Protect critical sections | Lock-free operations | One-time results | Thread group sync |
| **Thread State** | Sleeping when waiting | Running or blocked | Usually spinning | Sleeping when waiting | Sleeping at rendezvous point |
| **CPU Usage** | Low (threads sleep) | High if polling | High if spinning | Low | Low |
| **Reusability** | Reusable | Reusable | Reusable | One-time use | Depends on type |
| **Wake Latency** | 1-10μs | 1-10μs | 10-50ns | 1-10μs | 1-10μs |

**Choose Condition Variables When**:
- Threads need to wait for unpredictable events
- Wait times could be long (milliseconds or more)
- CPU efficiency is important
- Complex conditions need to be checked
- Producer-consumer or task queue patterns

### Futures & Promises

Futures provide a way to retrieve results from asynchronous operations:

```cpp
#include <future>

// Using async directly
std::future<int> fut = std::async(std::launch::async, []{ 
    // Expensive computation
    std::this_thread::sleep_for(std::chrono::seconds(2));
    return 42; 
});

// Do other work while computation happens

// Get result (blocks if not ready)
int result = fut.get(); // Blocks until result is available
```

#### Promise/Future Pairs

```cpp
// Manual promise/future
std::promise<std::string> promise;
std::future<std::string> future = promise.get_future();

// Pass the promise to another thread
std::thread worker([&promise]{
    try {
        std::string result = perform_task();
        promise.set_value(result);  // Set success value
    } catch(...) {
        promise.set_exception(std::current_exception()); // Forward exception
    }
});

// In main thread
try {
    std::string result = future.get();
    use_result(result);
} catch(const std::exception& e) {
    // Handle exception from worker thread
    std::cerr << "Exception from worker: " << e.what() << std::endl;
}

worker.join();
```

#### Shared Futures

```cpp
std::promise<int> promise;
std::shared_future<int> shared_future = promise.get_future().share();

// Multiple threads can now wait on the same future
std::thread t1([shared_future]{ 
    int value = shared_future.get(); // Copy 1 
    use_value(value);
});

std::thread t2([shared_future]{ 
    int value = shared_future.get(); // Copy 2
    use_another_way(value);
});

// Set the value once for all waiters
promise.set_value(42);
t1.join();
t2.join();
```

#### std::async Details

```cpp
// Four possible launch policies
auto f1 = std::async(std::launch::async, func);    // Run in new thread
auto f2 = std::async(std::launch::deferred, func); // Lazy evaluation on get()
auto f3 = std::async(func);                        // Implementation choice
auto f4 = std::async(std::launch::async | std::launch::deferred, func); // Same as f3
```

### One-time Initialization

`std::call_once` ensures a function is called exactly once, even with multiple threads:

```cpp
#include <mutex>
std::once_flag init_flag;
DB* database = nullptr;

void init_database() {
    database = new DB("config.ini");
    database->connect();
}

void thread_func() {
    // Multiple threads can call this safely
    std::call_once(init_flag, init_database);
    
    // Use database knowing it's initialized exactly once
    database->query("SELECT * FROM users");
}
```

#### How It Works

1. **Atomic State Transitions**:
   - `once_flag` contains an atomic state (not_started → in_progress → complete)
   - First thread sets state to in_progress and executes the function
   - Other threads see in_progress and wait
   - When function completes, state changes to complete
   - Waiting threads continue without executing function

2. **Implementation Detail**:
   ```cpp
   // Simplified internal call_once implementation
   template<typename Callable, typename... Args>
   void call_once(once_flag& flag, Callable&& f, Args&&... args) {
       if (flag.test_and_set(std::memory_order_acquire) == false) {
           try {
               f(std::forward<Args>(args)...);
               flag.set_initialized(std::memory_order_release);
           } catch (...) {
               flag.reset(std::memory_order_release);
               throw;
           }
       } else {
           flag.wait_for_initialization();
       }
   }
   ```

3. **Memory Ordering**:
   - Provides acquire semantics for callers after initialization
   - Ensures proper happens-before relationships with initialized data

#### Common Usage Patterns

1. **Singleton Initialization**:
   ```cpp
   class Singleton {
   public:
       static Singleton& instance() {
           static std::once_flag init_flag;
           static Singleton* instance_ptr = nullptr;
           
           std::call_once(init_flag, [](){
               instance_ptr = new Singleton();
           });
           
           return *instance_ptr;
       }
   
   private:
       Singleton() = default;
   };
   ```

2. **Lazy Resource Initialization**:
   ```cpp
   class ExpensiveResource {
       std::once_flag init_flag;
       Resource* resource = nullptr;
       
   public:
       Resource* get() {
           std::call_once(init_flag, [this]{
               resource = new Resource();
               resource->load_data();
           });
           return resource;
       }
   };
   ```

3. **Thread-Safe Static Initialization (C++11)**:
   ```cpp
   static ExpensiveResource& getResource() {
       // Guaranteed thread-safe by C++11 standard
       static ExpensiveResource resource;
       return resource;
   }
   ```

### Barriers & Latches (C++20)

C++20 introduced barriers and latches for coordinating groups of threads:

#### Latches (One-time Coordination)

```cpp
#include <latch>

// Create a latch for 3 threads
std::latch completion_latch(3);

void worker(int id) {
    // Do work
    std::cout << "Worker " << id << " completed\n";
    
    // Signal completion
    completion_latch.count_down();
    // or completion_latch.arrive();
}

// Main thread
void coordinator() {
    // Start workers
    std::thread t1(worker, 1);
    std::thread t2(worker, 2);
    std::thread t3(worker, 3);
    
    // Wait for all to complete
    completion_latch.wait();
    
    std::cout << "All workers completed\n";
    
    t1.join(); t2.join(); t3.join();
}
```

A latch is a single-use, countdown synchronization point. Once the count reaches zero, it remains zero forever.

#### Barriers (Reusable Coordination Point)

```cpp
#include <barrier>

// Create a barrier for 3 threads with completion function
std::barrier sync_point(3, []{ std::cout << "Phase completed\n"; });

void worker(int id) {
    // Phase 1
    std::cout << "Worker " << id << " completed phase 1\n";
    sync_point.arrive_and_wait();
    
    // Phase 2
    std::cout << "Worker " << id << " completed phase 2\n";
    sync_point.arrive_and_wait();
    
    // Phase 3
    std::cout << "Worker " << id << " completed phase 3\n";
    sync_point.arrive_and_wait();
}
```

A barrier is a reusable synchronization point. Each time all threads arrive, the count resets, and a new cycle begins.

#### Implementation Details

1. **Latches**:
   - One-time use
   - Simple countdown, no per-cycle logic
   - `std::latch` cannot be reset after reaching zero
   - More efficient than a condition variable for simple countdowns

2. **Barriers**:
   - Multi-phase, reusable
   - Optional completion function runs on one thread when all arrive
   - Automatically resets after each phase
   - More expressive than condition variables for phased algorithms

3. **Performance**:
   - Both are optimized for their use cases
   - Often implemented with atomics and OS waiting primitives
   - Lower overhead than equivalent condition variable code

## Performance Considerations

### Synchronization Mechanism Performance

| Mechanism | Uncontended Cost | Contended Cost | CPU While Waiting |
|-----------|------------------|----------------|-------------------|
| Direct atomic | 20-50 cycles | 100-200 cycles | 100% if spinning |
| Mutex lock+unlock | 100-200 cycles | 5,000-15,000+ cycles | ~0% (sleeps) |
| Condition variable | Similar to mutex | Context switch + wakeup | ~0% (sleeps) |
| Futures/promises | Similar to mutex | Context switch + wakeup | ~0% (sleeps) |
| Barriers/latches | ~50-150 cycles | Context switch + wakeup | ~0% (sleeps) |

### Real-world Performance Hierarchy (fastest to slowest)

1. **Non-atomic operations**: baseline (1×)
2. **Relaxed atomics**: ~1.5-3× slower than non-atomic
3. **Acquire/release atomics**: ~2-5× slower than non-atomic
4. **Sequential consistency**: ~3-10× slower than non-atomic
5. **Uncontended mutex**: ~10-50× slower than non-atomic
6. **Contended mutex**: ~100-10,000× slower than non-atomic

### Performance Pitfalls

1. **False Sharing**:
   - Multiple threads accessing different variables on the same cache line
   - Cache line invalidation creates significant performance degradation
   - Use `alignas(64)` to ensure separate cache lines

2. **Lock Contention**:
   - Break large locks into smaller, more granular locks
   - Keep critical sections as short as possible
   - Consider reader-writer locks for read-heavy workloads

3. **Thread Oversubscription**:
   - Creating too many threads (more than cores) hurts performance
   - Use thread pools to limit the number of active threads

4. **Memory Ordering Overkill**:
   - Using sequential consistency when relaxed would suffice
   - Over-synchronizing with too many barriers
   - Not batching operations under a single fence

5. **NUMA Effects**:
   - Crossing NUMA nodes (multiple CPU sockets) adds ~3-5× more overhead
   - Try to keep related data and threads on the same NUMA node

### Optimizing Strategies

1. **Lock-free Reads with Mutex-protected Writes**:
   ```cpp
   class Cache {
       std::mutex write_mutex;
       std::atomic<bool> data_ready{false};
       ExpensiveData data;
   public:
       void update() {
           std::lock_guard<std::mutex> lock(write_mutex);
           data = compute_new_data();
           data_ready.store(true, std::memory_order_release);
       }
       
       ExpensiveData read() {
           if (data_ready.load(std::memory_order_acquire)) {
               return data; // Lock-free fast path when data is ready
           } else {
               std::lock_guard<std::mutex> lock(write_mutex);
               return data;
           }
       }
   };
   ```

2. **Batching Critical Sections**:
   ```cpp
   // BAD: Multiple small critical sections
   for (auto& item : items) {
       lock.lock();
       process(item);
       lock.unlock();
   }
   
   // GOOD: One larger critical section
   lock.lock();
   for (auto& item : items) {
       process(item);
   }
   lock.unlock();
   ```

3. **Move Expensive Operations Outside Critical Sections**:
   ```cpp
   // BAD: Long critical section
   std::lock_guard<std::mutex> lock(mutex);
   auto result = expensive_calculation(data);
   update_shared_data(result);
   
   // GOOD: Minimize critical section
   auto local_copy = [&]{
       std::lock_guard<std::mutex> lock(mutex);
       return data;
   }();
   
   auto result = expensive_calculation(local_copy);
   
   {
       std::lock_guard<std::mutex> lock(mutex);
       update_shared_data(result);
   }
   ```

4. **Read-Write Lock for Read-heavy Workloads**:
   ```cpp
   std::shared_mutex rw_mutex;
   
   // Reader (multiple can run concurrently)
   void read() {
       std::shared_lock<std::shared_mutex> lock(rw_mutex);
       // Read-only operations
   }
   
   // Writer (exclusive access)
   void write() {
       std::unique_lock<std::shared_mutex> lock(rw_mutex);
       // Update operations
   }
   ```

5. **Avoid Fine-grained Atomics in a Loop**:
   ```cpp
   // BAD: Lots of atomic operations
   for (int i = 0; i < 1000; i++) {
       counter.fetch_add(1, std::memory_order_relaxed);
   }
   
   // GOOD: Single atomic operation
   counter.fetch_add(1000, std::memory_order_relaxed);
   ```

