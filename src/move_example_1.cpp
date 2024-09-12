#include <iostream>
#include <memory>
#include <string>
#include <vector>

class LargeResource {
   public:
    LargeResource(size_t size) : data(new char[size]), size(size) {
        std::cout << "LargeResource constructed with size " << size << std::endl;
    }
    ~LargeResource() { delete[] data; }

    // Disable copy operations
    LargeResource(const LargeResource&) = delete;
    LargeResource& operator=(const LargeResource&) = delete;

    // Custom move operations
    LargeResource(LargeResource&& other) noexcept : data(other.data), size(other.size) {
        other.data = nullptr;
        other.size = 0;
    }

    LargeResource& operator=(LargeResource&& other) noexcept {
        if (this != &other) {
            delete[] data;
            data = other.data;
            size = other.size;
            other.data = nullptr;
            other.size = 0;
        }
        return *this;
    }

    size_t getSize() const { return size; }

   private:
    char* data;
    size_t size;
};

class ResourceManager {
   private:
    std::unique_ptr<LargeResource> resource;
    std::vector<std::string> metadata;

   public:
    ResourceManager(size_t size, std::initializer_list<std::string> meta)
        : resource(std::make_unique<LargeResource>(size)), metadata(meta) {
        std::cout << "ResourceManager created with resource size " << size << std::endl;
    }

    // Move constructor
    ResourceManager(ResourceManager&& other) noexcept
        : resource(std::move(other.resource)), metadata(std::move(other.metadata)) {
        std::cout << "Move constructor called" << std::endl;
    }

    // Move assignment operator
    ResourceManager& operator=(ResourceManager&& other) noexcept {
        if (this != &other) {
            resource = std::move(other.resource);
            metadata = std::move(other.metadata);
            std::cout << "Move assignment operator called" << std::endl;
        }
        return *this;
    }

    void printState() const {
        std::cout << "Resource state:" << std::endl;
        std::cout << "  LargeResource: "
                  << (resource ? "valid (size: " + std::to_string(resource->getSize()) + ")"
                               : "null")
                  << std::endl;
        std::cout << "  metadata: "
                  << (metadata.empty()
                          ? "empty"
                          : "contains " + std::to_string(metadata.size()) + " elements")
                  << std::endl;
    }
};

int main() {
    ResourceManager rm1(1000000, {"data1", "data2", "data3"});
    std::cout << "Original rm1:" << std::endl;
    rm1.printState();

    // Move rm1 to rm2
    ResourceManager rm2 = std::move(rm1);
    std::cout << "\nAfter move:" << std::endl;
    std::cout << "rm2 (moved-to):" << std::endl;
    rm2.printState();
    std::cout << "rm1 (moved-from):" << std::endl;
    rm1.printState();

    // Create a new resource and move-assign it to rm2
    ResourceManager rm3(2000000, {"data4", "data5", "data6", "data7"});
    rm2 = std::move(rm3);
    std::cout << "\nAfter move assignment:" << std::endl;
    std::cout << "rm2 (moved-to):" << std::endl;
    rm2.printState();
    std::cout << "rm3 (moved-from):" << std::endl;
    rm3.printState();

    return 0;
}
