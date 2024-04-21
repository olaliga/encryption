cmake_minimum_required(VERSION 3.16) # CMake_最低版本_需求

project("decrypt") # 建立一個CMake專案，並給予專案名稱
set(CMAKE_CXX_STANDARD 17)

# 找到 OpenSSL
find_package(OpenSSL REQUIRED)

# 將 OpenSSL 標頭文件添加到 include 目錄
include_directories(${OPENSSL_INCLUDE_DIR})


set(CMAKE_PREFIX_PATH "/Users/huangshijie/Desktop/test_libtorch/libtorch/share/cmake/Torch")
find_package(Torch REQUIRED)


# 將所有源文件添加到變數 SRC_FILES
file(GLOB SRC_FILES "example_decrypt_torch.cpp")

# 添加執行檔
add_executable(example_decrypt_torch ${SRC_FILES})

# 链接 OpenSSL 库
target_link_libraries(example_decrypt_torch ${OPENSSL_LIBRARIES})
target_link_libraries(example_decrypt_torch ${TORCH_LIBRARIES})