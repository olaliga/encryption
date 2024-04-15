cmake_minimum_required(VERSION 3.16) # CMake_最低版本_需求

project("decrypt") # 建立一個CMake專案，並給予專案名稱


# 找到 OpenSSL
find_package(OpenSSL REQUIRED)

# 將 OpenSSL 標頭文件添加到 include 目錄
include_directories(${OPENSSL_INCLUDE_DIR})

# 將所有源文件添加到變數 SRC_FILES
file(GLOB SRC_FILES "encrypt.cpp")

# 添加執行檔
add_executable(encrypt ${SRC_FILES})

# 链接 OpenSSL 库
target_link_libraries(encrypt ${OPENSSL_LIBRARIES})
