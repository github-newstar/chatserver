#!/bin/bash
# 运行测试并将输出同时写入标准输出和日志文件
echo "Starting tests at $(date)"
cd /root/chatServerTest
./RUN_TEST 2>&1 | tee /logs/test_output.log
EXIT_CODE=${PIPESTATUS[0]}
echo "Tests completed with status $EXIT_CODE at $(date)"
# 将退出状态码写入日志
echo "Exit code: $EXIT_CODE" >> /logs/test_output.log
# 使用实际的测试退出码作为容器的退出码
exit $EXIT_CODE