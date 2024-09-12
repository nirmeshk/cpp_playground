#include <chrono>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

class LargeResource {
   public:
    LargeResource(size_t size = 1000000) : data_(size, 'a') {}

    LargeResource(const LargeResource& other) : data_(other.data_) {
        // std::cout << "Copy constructed" << std::endl;
    }

    LargeResource(LargeResource&& other) noexcept : data_(std::move(other.data_)) {
        // std::cout << "Move constructed" << std::endl;
    }

    LargeResource& operator=(const LargeResource& other) {
        if (this != &other) {
            data_ = other.data_;
            // std::cout << "Copy assigned" << std::endl;
        }
        return *this;
    }

    LargeResource& operator=(LargeResource&& other) noexcept {
        if (this != &other) {
            data_ = std::move(other.data_);
            // std::cout << "Move assigned" << std::endl;
        }
        return *this;
    }

    size_t size() const { return data_.size(); }

   private:
    std::string data_;
};

template <typename Func>
double measureTime(Func&& func) {
    auto start = std::chrono::high_resolution_clock::now();
    func();
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<double, std::milli>(end - start).count();
}

int main() {
    const int num_iterations = 1000;

    std::cout << std::fixed << std::setprecision(2);

    // Test vector insertions
    {
        double copy_time = measureTime([&]() {
            std::vector<LargeResource> vec;
            for (int i = 0; i < num_iterations; ++i) {
                LargeResource resource;
                vec.push_back(resource);  // Copy
            }
        });

        double move_time = measureTime([&]() {
            std::vector<LargeResource> vec;
            for (int i = 0; i < num_iterations; ++i) {
                LargeResource resource;
                vec.push_back(std::move(resource));  // Move
            }
        });

        std::cout << "Vector insertions:\n";
        std::cout << "Copy time: " << copy_time << " ms\n";
        std::cout << "Move time: " << move_time << " ms\n";
        std::cout << "Move is " << (copy_time / move_time) << " times faster\n\n";
    }

    // Test assignments
    {
        double copy_time = measureTime([&]() {
            LargeResource a, b;
            for (int i = 0; i < num_iterations; ++i) {
                a = b;  // Copy assignment
            }
        });

        double move_time = measureTime([&]() {
            LargeResource a, b;
            for (int i = 0; i < num_iterations; ++i) {
                a = std::move(b);     // Move assignment
                b = LargeResource();  // Reset b for fair comparison
            }
        });

        std::cout << "Assignments:\n";
        std::cout << "Copy time: " << copy_time << " ms\n";
        std::cout << "Move time: " << move_time << " ms\n";
        std::cout << "Move is " << (copy_time / move_time) << " times faster\n";
    }

    return 0;
}
