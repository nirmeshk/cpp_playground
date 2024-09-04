#include "gtest/gtest.h"
#include "rpc.cpp"

using namespace std;

TEST(EchoTest, testJsonParsing) {
    std::string json_data_str = R"({"src": "n1", "dest": "c1", "body": {"type": "init"}})";
    json j2 = json::parse(json_data_str);
    Message message(j2);

    EXPECT_EQ(message.src, "n1");
    EXPECT_EQ(message.dest, "c1");
    EXPECT_TRUE(message.body.has_value());
    EXPECT_EQ(message.body->type, "init");
}
