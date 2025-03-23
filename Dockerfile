#base-image编译了基础的环境，已经发布到dockerhub省得每次都要编译
#--------------------编译chatServer
#chatServer的编译
FROM jojo114514/base-image AS builder


#配置gTest
WORKDIR /app
RUN wget https://github.com/google/googletest/releases/download/v1.16.0/googletest-1.16.0.tar.gz && \
    tar -zxvf googletest-1.16.0.tar.gz && \
    cd googletest-1.16.0 && \
    mkdir build && \
    cd build && \
    cmake .. -G Ninja -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Release && \
    ninja && \
    ninja install && \
    rm -rf /app/*

WORKDIR /root/code/chatServer

#COPY目录下的最新代码
COPY . .
#构建
RUN mkdir build && \
    cd build && \
    cmake .. -G Ninja -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Release && \
    ninja 

#-------------------构建最终发布镜像
FROM ubuntu:22.04 AS final
WORKDIR /root
RUN apt-get update && apt-get install -y --no-install-recommends \
    ca-certificates \
    libicu70 \
    libssl3 \
    libstdc++6 \
    libmysqlclient21 \
    && rm -rf /var/lib/apt/lists/*

# 从构建阶段复制必要的库文件
# Boost库
COPY --from=builder /usr/lib/libboost_*.so* /usr/lib/

# JsonCpp库
COPY --from=builder /usr/lib/x86_64-linux-gnu/libjsoncpp* /usr/lib

# gRPC及相关库
COPY --from=builder /usr/lib/libgrpc*.so* /usr/lib/
COPY --from=builder /usr/lib/libprotobuf.so* /usr/lib/
COPY --from=builder /usr/lib/libabsl*.so* /usr/lib/
COPY --from=builder /usr/lib/libupb*.so* /usr/lib/
COPY --from=builder /usr/lib/libaddress_sorting.so* /usr/lib/
COPY --from=builder /usr/lib/libgpr.so* /usr/lib/

# Redis相关库
COPY --from=builder /usr/lib/x86_64-linux-gnu/libredis* /usr/lib/
COPY --from=builder /usr/lib/x86_64-linux-gnu/libhiredis* /usr/lib/
COPY --from=builder /usr/lib/libredis++.so* /usr/lib/

# MySQL C++ 连接器
COPY --from=builder /usr/lib64/libmysqlcppconn*.so* /usr/lib64/

# 拷贝二进制文件
COPY --from=builder /root/code/chatServer/build/chatServer /root/chatServer/chatServer
COPY --from=builder /root/code/chatServer/build/config.ii  /root/chatServer/config.ii

# 更新动态链接器配置
RUN echo "/usr/lib64" >> /etc/ld.so.conf.d/mysql-connector-cpp.conf && \
    ldconfig

WORKDIR /root/chatServer
CMD ["/root/chatServer/chatServer"]


FROM final AS test
WORKDIR /root/code/chatServer
COPY --from=builder /root/code/chatServer/build/RUN_TEST /root/chatServer/RUN_TEST
COPY ./test/test.sh .
RUN chmod +x test.sh

CMD [ "/root/chatServer/test.sh" ]
