#include<iostream>
#include<string>
#include <optional>
#include <nlohmann/json.hpp>

using namespace std;
using json = nlohmann::json;

struct Body {
    std::string type;
    Body(const json& j) : type(j["type"].get<std::string>()) {}
};

class Message {
public:
    string src;
    string dest;
    optional<Body> body;

    Message(const json& j) {
        src = j["src"].get<std::string>();
        dest = j["dest"].get<std::string>();
        if (j.contains("body")) {
            body = Body(j["body"]);
        }
    }
};
