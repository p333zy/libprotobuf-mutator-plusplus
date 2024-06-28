#include <iostream>
#include <fcntl.h>
#include <fstream>
#include <gtest/gtest.h>
#include <google/protobuf/text_format.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>

#include "proto/test.pb.h"
#include "trimming.hh"
#include "utils.hh"

namespace lpmpp::test {

using namespace google::protobuf;

void LoadFixture(const char *path, Message& msg) {
  std::ifstream ifs;
  ifs.open(path, std::ifstream::in);
  if (!ifs.is_open()) {
    std::cerr << "Failed to open: " << path << "\n";
    std::abort();
  }

  io::IstreamInputStream input(&ifs);
  if (!google::protobuf::TextFormat::Parse(&input, &msg)) {
    std::cerr << "Failed to parse: " << path << "\n";
    std::abort();
  }
}

class TrimmerTest : public testing::Test {
protected:
  TestMsg msg1;
  TestMsg msg2;

  TrimmerTest() {
    LoadFixture("../tests/fixtures/testmsg1.pb.txt", msg1);
    LoadFixture("../tests/fixtures/testmsg2.pb.txt", msg2);
  }
};

TEST_F(TrimmerTest, TrimsNodes) {
  std::unique_ptr<Message> msg = std::make_unique<TestMsg>();
  msg->CopyFrom(msg1);

  Trimmer trimmer(std::move(msg));

  const TestMsg *testmsg = static_cast<const TestMsg *>(trimmer.message());
  ASSERT_EQ(testmsg->nested_size(), 1);

  const NestedTestMsg *nestedmsg = &testmsg->nested()[0];
  ASSERT_TRUE(nestedmsg->has_str1());
  ASSERT_TRUE(nestedmsg->has_blob2());
  ASSERT_TRUE(nestedmsg->has_integer());

  trimmer.TrimOne();
  trimmer.TrimOne();
  trimmer.TrimOne();
  ASSERT_FALSE(nestedmsg->has_str2());
  ASSERT_FALSE(nestedmsg->has_blob2());
  ASSERT_FALSE(nestedmsg->has_integer());

  trimmer.Revert();
  nestedmsg = &testmsg->nested()[0];
  ASSERT_TRUE(nestedmsg->has_str2());

  trimmer.TrimOne();
  ASSERT_EQ(testmsg->nested().size(), 0);

  trimmer.Revert();
  ASSERT_EQ(testmsg->nested().size(), 1);
  nestedmsg = &testmsg->nested()[0];
  ASSERT_TRUE(nestedmsg->has_str2());
}

TEST_F(TrimmerTest, ShufflesNodes) {
  std::unique_ptr<Message> msg = std::make_unique<TestMsg>();
  msg->CopyFrom(msg2);

  Trimmer trimmer(std::move(msg));

  const TestMsg *testmsg = static_cast<const TestMsg *>(trimmer.message());
  ASSERT_EQ(testmsg->nested_size(), 4);

  // Trim all leaf fields
  trimmer.TrimOne();
  trimmer.TrimOne();
  trimmer.TrimOne();
  trimmer.TrimOne();

  ASSERT_EQ(testmsg->nested_size(), 4);
  ASSERT_STREQ(testmsg->nested()[0].str1().c_str(), "a");
  ASSERT_STREQ(testmsg->nested()[1].str1().c_str(), "b");
  ASSERT_STREQ(testmsg->nested()[2].str1().c_str(), "c");
  ASSERT_STREQ(testmsg->nested()[3].str1().c_str(), "d");

  trimmer.TrimOne();
  ASSERT_EQ(testmsg->nested_size(), 3);
  ASSERT_STREQ(testmsg->nested()[0].str1().c_str(), "a");
  ASSERT_STREQ(testmsg->nested()[1].str1().c_str(), "b");
  ASSERT_STREQ(testmsg->nested()[2].str1().c_str(), "c");

  trimmer.Revert();
  testmsg = static_cast<const TestMsg *>(trimmer.message());
  ASSERT_EQ(testmsg->nested_size(), 4);
  ASSERT_STREQ(testmsg->nested()[0].str1().c_str(), "a");
  ASSERT_STREQ(testmsg->nested()[1].str1().c_str(), "b");
  ASSERT_STREQ(testmsg->nested()[2].str1().c_str(), "c");
  ASSERT_STREQ(testmsg->nested()[3].str1().c_str(), "d");

  trimmer.TrimOne();
  ASSERT_EQ(testmsg->nested_size(), 3);
  ASSERT_STREQ(testmsg->nested()[0].str1().c_str(), "a");
  ASSERT_STREQ(testmsg->nested()[1].str1().c_str(), "b");
  ASSERT_STREQ(testmsg->nested()[2].str1().c_str(), "d");

  trimmer.TrimOne();
  ASSERT_EQ(testmsg->nested_size(), 2);
  ASSERT_STREQ(testmsg->nested()[0].str1().c_str(), "a");
  ASSERT_STREQ(testmsg->nested()[1].str1().c_str(), "d");

  trimmer.Revert();
  testmsg = static_cast<const TestMsg *>(trimmer.message());
  ASSERT_EQ(testmsg->nested_size(), 3);
  ASSERT_STREQ(testmsg->nested()[0].str1().c_str(), "a");
  ASSERT_STREQ(testmsg->nested()[1].str1().c_str(), "b");
  ASSERT_STREQ(testmsg->nested()[2].str1().c_str(), "d");

  trimmer.TrimOne();
  ASSERT_EQ(testmsg->nested_size(), 2);
  ASSERT_STREQ(testmsg->nested()[0].str1().c_str(), "b");
  ASSERT_STREQ(testmsg->nested()[1].str1().c_str(), "d");

  trimmer.Revert();
  ASSERT_STREQ(testmsg->nested()[0].str1().c_str(), "a");
  ASSERT_STREQ(testmsg->nested()[1].str1().c_str(), "b");
  ASSERT_STREQ(testmsg->nested()[2].str1().c_str(), "d");

  ASSERT_TRUE(trimmer.trim_type() == TrimType::STRINGS);
}

TEST_F(TrimmerTest, TrimsStrings) {
  std::unique_ptr<Message> msg = std::make_unique<TestMsg>();
  msg->CopyFrom(msg1);

  Trimmer trimmer(std::move(msg));
  trimmer.TrimOne();
  trimmer.Revert();
  trimmer.TrimOne();
  trimmer.Revert();
  trimmer.TrimOne();
  trimmer.Revert();
  trimmer.TrimOne();
  trimmer.Revert();

  const TestMsg *testmsg = static_cast<const TestMsg *>(trimmer.message());
  ASSERT_EQ(testmsg->nested().size(), 1);
  ASSERT_EQ(trimmer.trim_type(), TrimType::STRINGS);
  
  const NestedTestMsg *nestedmsg = &testmsg->nested()[0];
  ASSERT_STREQ(nestedmsg->blob2().c_str(), "abcdefg");

  trimmer.TrimOne();
  ASSERT_STREQ(nestedmsg->blob2().c_str(), "abc");

  trimmer.Revert();
  nestedmsg = &testmsg->nested()[0];
  ASSERT_STREQ(nestedmsg->blob2().c_str(), "abcdefg");
  ASSERT_EQ(nestedmsg->blob1().size(), 2932);
  trimmer.TrimOne();
  ASSERT_EQ(nestedmsg->blob1().size(), 2932 - 256);

  int i = 1;
  while (nestedmsg->blob1().size() > 5) {
    trimmer.TrimOne();
    i++;
  }
  ASSERT_EQ(i, 56);
  ASSERT_STREQ(nestedmsg->blob1().c_str(), "zzzz");

  trimmer.Revert();
  nestedmsg = &testmsg->nested()[0];
  ASSERT_STREQ(nestedmsg->blob1().c_str(), "zzzzbbbb");

  trimmer.TrimOne();
  ASSERT_STREQ(nestedmsg->str2().c_str(), "Hello w");

  trimmer.Revert();
  nestedmsg = &testmsg->nested()[0];
  ASSERT_STREQ(nestedmsg->str2().c_str(), "Hello world");

  std::size_t len = nestedmsg->str1().size();
  ASSERT_EQ(len, 52);

  while (nestedmsg->str1().size() > 5) {
    trimmer.TrimOne();
    len -= 4;
    ASSERT_EQ(nestedmsg->str1().size(), len);
  } 

  ASSERT_STREQ(nestedmsg->str1().c_str(), "zzzz");
  trimmer.Revert();
  ASSERT_STREQ(nestedmsg->str1().c_str(), "zzzzaaaa");

  trimmer.TrimOne();
  trimmer.Revert();
  ASSERT_STREQ(nestedmsg->str1().c_str(), "zzzzaaaa");
  ASSERT_TRUE(trimmer.done());
}

};

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
