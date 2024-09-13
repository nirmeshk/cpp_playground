#include <algorithm>
#include <chrono>
#include <deque>
#include <iomanip>
#include <iostream>
#include <list>
#include <memory>
#include <random>
#include <vector>

using namespace std;

// A simple class to simulate some data and operations
class DataObject {
    std::vector<int> data;

   public:
    DataObject() : data(100) { std::fill(data.begin(), data.end(), 42); }
    void operation() {
        std::transform(data.begin(), data.end(), data.begin(), [](int n) { return n * 2; });
    }
};

template <typename T>
class ObjectPool {
    std::vector<std::unique_ptr<T>> pool;
    std::vector<size_t> free_indices;
    std::unordered_map<T*, size_t> object_indices;

   public:
    ObjectPool(size_t initial_size = 1000) {
        pool.reserve(initial_size);
        free_indices.reserve(initial_size);
        for (size_t i = 0; i < initial_size; ++i) {
            pool.push_back(std::make_unique<T>());
            free_indices.push_back(i);
            object_indices[pool[i].get()] = i;
        }
    }

    T* acquire() {
        if (free_indices.empty()) {
            size_t new_index = pool.size();
            pool.push_back(std::make_unique<T>());
            object_indices[pool.back().get()] = new_index;
            return pool.back().get();
        }
        size_t index = free_indices.back();
        free_indices.pop_back();
        return pool[index].get();
    }

    void release(T* obj) {
        auto it = object_indices.find(obj);
        if (it != object_indices.end()) {
            free_indices.push_back(it->second);
        }
    }

    ~ObjectPool() {
        // Clear the object_indices map to avoid accessing deleted memory
        object_indices.clear();
    }
};

// Timer utility
class Timer {
    std::chrono::high_resolution_clock::time_point start;

   public:
    Timer() : start(std::chrono::high_resolution_clock::now()) {}
    double elapsed() {
        auto end = std::chrono::high_resolution_clock::now();
        return std::chrono::duration<double, std::milli>(end - start).count();
    }
};

// Function to perform operations and measure time
template <typename Container>
double measure_operations(Container& container, int num_operations) {
    Timer timer;
    for (int i = 0; i < num_operations; ++i) {
        container[i % container.size()]->operation();
    }
    return timer.elapsed();
}

// Test function for dynamic allocation
double test_dynamic_allocation(int num_objects, int num_operations) {
    std::vector<std::unique_ptr<DataObject>> objects;
    objects.reserve(num_objects);

    Timer timer;
    for (int i = 0; i < num_objects; ++i) {
        objects.push_back(std::make_unique<DataObject>());
    }

    double creation_time = timer.elapsed();
    double operation_time = measure_operations(objects, num_operations);

    return creation_time + operation_time;
}

// Test function for object pooling
double test_object_pool(int num_objects, int num_operations) {
    ObjectPool<DataObject> pool(num_objects);
    std::vector<DataObject*> objects;
    objects.reserve(num_objects);

    Timer timer;
    for (int i = 0; i < num_objects; ++i) {
        objects.push_back(pool.acquire());
    }

    double creation_time = timer.elapsed();
    double operation_time = measure_operations(objects, num_operations);

    for (auto obj : objects) {
        pool.release(obj);
    }

    return creation_time + operation_time;
}

// Test function for std::vector
double test_vector(int num_objects, int num_operations) {
    Timer timer;
    std::vector<DataObject> objects(num_objects);
    double creation_time = timer.elapsed();

    std::vector<DataObject*> object_ptrs;
    object_ptrs.reserve(num_objects);
    for (auto& obj : objects) {
        object_ptrs.push_back(&obj);
    }

    double operation_time = measure_operations(object_ptrs, num_operations);
    return creation_time + operation_time;
}

// Test function for std::list
double test_list(int num_objects, int num_operations) {
    Timer timer;
    std::list<DataObject> objects(num_objects);
    double creation_time = timer.elapsed();

    std::vector<DataObject*> object_ptrs;
    object_ptrs.reserve(num_objects);
    for (auto& obj : objects) {
        object_ptrs.push_back(&obj);
    }

    double operation_time = measure_operations(object_ptrs, num_operations);
    return creation_time + operation_time;
}

// Test function for std::deque
double test_deque(int num_objects, int num_operations) {
    Timer timer;
    std::deque<DataObject> objects(num_objects);
    double creation_time = timer.elapsed();

    std::vector<DataObject*> object_ptrs;
    object_ptrs.reserve(num_objects);
    for (auto& obj : objects) {
        object_ptrs.push_back(&obj);
    }

    double operation_time = measure_operations(object_ptrs, num_operations);
    return creation_time + operation_time;
}

int main() {
    const int num_objects = 10000;
    const int num_operations = 10000;
    const int num_runs = 5;

    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Performance comparison (average of " << num_runs << " runs):\n";
    std::cout << "Objects: " << num_objects << ", Operations: " << num_operations << "\n\n";

    auto run_test = [num_runs, num_objects, num_operations](const std::string& name,
                                                            auto test_func) {
        double total_time = 0;
        for (int i = 0; i < num_runs; ++i) {
            total_time += test_func(num_objects, num_operations);
        }
        double avg_time = total_time / num_runs;
        std::cout << std::setw(20) << std::left << name << ": " << avg_time << " ms\n";
        return avg_time;
    };

    double dynamic_time = run_test("Dynamic Allocation", test_dynamic_allocation);
    double pool_time = run_test("Object Pool", test_object_pool);
    double vector_time = run_test("std::vector", test_vector);
    double list_time = run_test("std::list", test_list);
    double deque_time = run_test("std::deque", test_deque);

    std::cout << "\nRelative Performance (compared to Dynamic Allocation):\n";
    std::cout << std::setw(20) << std::left << "Object Pool" << ": " << (dynamic_time / pool_time)
              << "x faster\n";
    std::cout << std::setw(20) << std::left << "std::vector" << ": " << (dynamic_time / vector_time)
              << "x faster\n";
    std::cout << std::setw(20) << std::left << "std::list" << ": " << (dynamic_time / list_time)
              << "x faster\n";
    std::cout << std::setw(20) << std::left << "std::deque" << ": " << (dynamic_time / deque_time)
              << "x faster\n";

    return 0;
}
