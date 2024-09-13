#include <chrono>
#include <functional>
#include <iostream>
#include <memory>
#include <random>
#include <vector>

struct Data {
    int x, y, z;
    Data(int a, int b, int c) : x(a), y(b), z(c) {}
};

using microseconds = std::chrono::microseconds;
using high_resolution_clock = std::chrono::high_resolution_clock;
using time_point = high_resolution_clock::time_point;

const int SIZE = 1000000;         // 1 million elements
const int ACCESS_COUNT = 100000;  // 100k random accesses
const int ITERATIONS = 100;       // Number of iterations for averaging

// Function to get current time
time_point now() { return high_resolution_clock::now(); }

// Function to calculate duration in microseconds
long long duration(time_point start, time_point end) {
    return std::chrono::duration_cast<microseconds>(end - start).count();
}

// Function to create and populate vector with direct storage
std::vector<Data> createDirectVector() {
    std::vector<Data> vec;
    vec.reserve(SIZE);
    for (int i = 0; i < SIZE; ++i) {
        vec.emplace_back(i, i + 1, i + 2);
    }
    return vec;
}

// Function to create and populate vector with pointer-based storage
std::vector<std::unique_ptr<Data>> createIndirectVector() {
    std::vector<std::unique_ptr<Data>> vec;
    vec.reserve(SIZE);
    for (int i = 0; i < SIZE; ++i) {
        vec.push_back(std::make_unique<Data>(i, i + 1, i + 2));
    }
    return vec;
}

// Function to perform random access on direct storage
int accessDirectVector(const std::vector<Data>& vec, const std::vector<int>& indices) {
    int sum = 0;
    for (int index : indices) {
        const auto& data = vec[index];
        sum += data.x + data.y + data.z;
    }
    return sum;
}

// Function to perform random access on pointer-based storage
int accessIndirectVector(const std::vector<std::unique_ptr<Data>>& vec,
                         const std::vector<int>& indices) {
    int sum = 0;
    for (int index : indices) {
        const auto& data = vec[index];
        sum += data->x + data->y + data->z;
    }
    return sum;
}

// Function to generate random indices
std::vector<int> generateRandomIndices(int count, int max) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, max - 1);
    std::vector<int> indices(count);
    for (int& index : indices) {
        index = dis(gen);
    }
    return indices;
}

// Function to run benchmark
template <typename Func>
long long runBenchmark(Func func) {
    auto start = now();
    func();
    auto end = now();
    return duration(start, end);
}

int main() {
    long long direct_creation_total = 0, indirect_creation_total = 0;
    long long direct_access_total = 0, indirect_access_total = 0;

    for (int iter = 0; iter < ITERATIONS; ++iter) {
        // Benchmark creation
        direct_creation_total += runBenchmark(createDirectVector);
        indirect_creation_total += runBenchmark(createIndirectVector);

        // Create vectors for access benchmark
        auto direct_vec = createDirectVector();
        auto indirect_vec = createIndirectVector();
        auto indices = generateRandomIndices(ACCESS_COUNT, SIZE);

        // Benchmark access
        direct_access_total += runBenchmark([&]() { accessDirectVector(direct_vec, indices); });
        indirect_access_total +=
            runBenchmark([&]() { accessIndirectVector(indirect_vec, indices); });

        // Progress indicator
        std::cout << "." << std::flush;
    }
    std::cout << std::endl;

    // Print results
    auto print_results = [](const std::string& operation, long long direct, long long indirect) {
        std::cout << operation << " benchmark results:\n"
                  << "  Direct storage:   " << direct / ITERATIONS << " microseconds\n"
                  << "  Indirect storage: " << indirect / ITERATIONS << " microseconds\n"
                  << "  Difference:       " << (indirect - direct) / ITERATIONS
                  << " microseconds\n\n";
    };

    print_results("Creation", direct_creation_total, indirect_creation_total);
    print_results("Access", direct_access_total, indirect_access_total);

    return 0;
}
