#include <gtest/gtest.h>
#include "../src/RedisMgr.hpp"
#include <string>
#include<sw/redis++/redis++.h>

class RedisMgrTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Redis测试前准备工作
        // 确保Redis服务运行中
    }
    
    void TearDown() override {
        // 清理测试数据
        RedisMgr::GetInstance()->Del("test_key");
    }
};

// 测试Set和Get方法
TEST_F(RedisMgrTest, SetAndGet) {
    auto redis = RedisMgr::GetInstance();
    
    // 设置一个测试键值
    bool set_result = redis->Set("test_key", "test_value");
    EXPECT_TRUE(set_result) << "Set操作失败";
    
    // 获取并验证值
    std::string value;
    bool get_result = redis->Get("test_key", value);
    
    EXPECT_TRUE(get_result) << "Get操作失败";
    EXPECT_EQ(value, "test_value") << "获取的值与设置的不匹配";
}

// 测试ExistsKey方法
TEST_F(RedisMgrTest, KeyExists) {
    auto redis = RedisMgr::GetInstance();
    
    // 先设置一个测试键
    redis->Set("test_exists_key", "some_value");
    
    // 测试键是否存在
    bool exists = redis->ExistsKey("test_exists_key");
    EXPECT_TRUE(exists) << "应该存在的键未检测到";
    
    // 测试不存在的键
    exists = redis->ExistsKey("nonexistent_key");
    EXPECT_FALSE(exists) << "不存在的键被错误检测为存在";
    
    // 清理
    redis->Del("test_exists_key");
}

// 测试Del方法
TEST_F(RedisMgrTest, DeleteKey) {
    auto redis = RedisMgr::GetInstance();
    
    // 先设置一个测试键
    redis->Set("test_delete_key", "to_be_deleted");
    
    // 删除键
    bool del_result = redis->Del("test_delete_key");
    EXPECT_TRUE(del_result) << "删除操作失败";
    
    // 验证键已被删除
    std::string value;
    bool get_result = redis->Get("test_delete_key", value);
    EXPECT_FALSE(get_result) << "键应该已被删除";
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}