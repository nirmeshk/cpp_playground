#include <iostream>
#include <nlohmann/json.hpp>
#include <string>

using namespace std;

using json = nlohmann::json;


int main() {
    //std::cout << " Echo example start";
    string nodeId;
    bool c = true;

    while (c) {
        string input;

        // Note: Made a mistake here in first iteration
        // Was using a regular cin >> input
        // which was gettting terminated on spaces
        getline(cin, input);

        if (input.empty()) {
            return 0;
        }

        json j2 = json::parse(input);
        string type = j2["body"]["type"];

        if(type ==  "echo") {
            int messageId = j2["body"]["msg_id"];
            string echo = j2["body"]["echo"];
            json output = {
                {"src", nodeId},
                {"dest", j2["src"]},
                {"body", {
                    {"type", "echo_ok"},
                    {"in_reply_to", messageId},
                    {"msg_id", messageId},
                    {"echo", echo}
                }}
            };

            cout << output.dump() << endl;
        } else if (type == "init") {
            nodeId = j2["body"]["node_id"];
            string dest = j2["src"];
            int messageId = j2["body"]["msg_id"];

            json output = {
                {"src", nodeId},
                {"dest", dest},
                {"body", {
                    {"type", "init_ok"},
                    {"in_reply_to", messageId}
                }}
            };

            cout << output.dump() << endl;
        }
    }
}
