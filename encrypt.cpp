#include <iostream>
#include <openssl/aes.h>
#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <openssl/rand.h>
#include <openssl/bio.h>
#include <openssl/sha.h>
#include <openssl/buffer.h>
#include <fstream>
#include <ctime>
#include <iomanip>

// 生成32位随机数作为密钥
std::string generate_key() {
    unsigned char key[32];
    RAND_bytes(key, sizeof(key));
    return std::string(reinterpret_cast<char*>(key), sizeof(key));
}

// 使用PKCS7填充方式对数据进行填充
std::string pkcs7_pad(const std::string& data, int block_size) {
    int padding = block_size - (data.size() % block_size);
    std::string padded_data = data;
    padded_data.append(padding, static_cast<char>(padding));
    return padded_data;
}

// AES CBC 加密
std::string aes_cbc_encrypt(const std::string& data, const std::string& key, const std::string& iv) {
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        return "";
    }

    if (EVP_EncryptInit_ex(ctx, EVP_aes_128_cbc(), NULL, reinterpret_cast<const unsigned char*>(key.c_str()), reinterpret_cast<const unsigned char*>(iv.c_str())) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return "";
    }

    std::string ciphertext;
    int len;
    int max_len = data.size() + AES_BLOCK_SIZE; // 额外空间用于填充
    ciphertext.resize(max_len);

    if (EVP_EncryptUpdate(ctx, reinterpret_cast<unsigned char*>(&ciphertext[0]), &len, reinterpret_cast<const unsigned char*>(data.c_str()), data.size()) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return "";
    }
    int ciphertext_len = len;

    if (EVP_EncryptFinal_ex(ctx, reinterpret_cast<unsigned char*>(&ciphertext[len]), &len) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return "";
    }
    ciphertext_len += len;

    EVP_CIPHER_CTX_free(ctx);
    ciphertext.resize(ciphertext_len);
    return ciphertext;
}

// 计算 HMAC
std::string calculate_hmac(const std::string& data, const std::string& key) {
    unsigned char* hmac_result = HMAC(EVP_sha256(), key.c_str(), key.size(), reinterpret_cast<const unsigned char*>(data.c_str()), data.size(), NULL, NULL);
    if (!hmac_result) {
        return "";
    }
    std::string hmac(reinterpret_cast<char*>(hmac_result), SHA256_DIGEST_LENGTH);
    return hmac;
}

// 将数据进行 base64 编码
std::string base64_encode(const std::string& data) {
    BIO* bio = BIO_new(BIO_s_mem());
    BIO* b64 = BIO_new(BIO_f_base64());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    bio = BIO_push(b64, bio);
    BIO_write(bio, data.c_str(), data.length());
    BIO_flush(bio);
    BUF_MEM* bufferPtr;
    BIO_get_mem_ptr(bio, &bufferPtr);
    std::string encoded_data(bufferPtr->data, bufferPtr->length);
    BIO_free_all(bio);
    return encoded_data;
}

// 保存数据到文件
void save_to_file(const std::string& data, const std::string& filename) {
    std::ofstream outfile(filename, std::ios::binary);
    if (outfile.is_open()) {
        outfile << data;
        outfile.close();
        std::cout << "Data saved to " << filename << std::endl;
    } else {
        std::cerr << "Failed to save data to " << filename << std::endl;
    }
}

int main() {
    // 生成密钥
    std::string key = generate_key();

    // 将密钥以 base64 编码保存到文件
    std::string encoded_key = base64_encode(key);
    save_to_file(encoded_key, "key_cpp.key");

    // 获取当前时间戳
    std::time_t current_time = std::time(nullptr);

    // 随机生成16位的IV
    unsigned char iv[AES_BLOCK_SIZE];
    RAND_bytes(iv, sizeof(iv));
    std::string iv_str(reinterpret_cast<char*>(iv), AES_BLOCK_SIZE);

    // 要加密的原始数据
    std::string original_data = "Hello, world!";

    // 使用PKCS7填充方式对数据进行填充
    std::string padded_data = pkcs7_pad(original_data, AES_BLOCK_SIZE);

    // AES CBC 加密
    std::string ciphertext = aes_cbc_encrypt(padded_data, key.substr(16), iv_str);

    // 合并数据
    std::string basic_parts = "\x80" + std::to_string(current_time) + iv_str + ciphertext;

    // 计算HMAC
    std::string hmac = calculate_hmac(basic_parts, key.substr(0, 16));

    // 合并加密数据和HMAC
    std::string encrypted_data = base64_encode(basic_parts + hmac);

    // 将加密后的数据以 base64 编码保存到文件
    save_to_file(encrypted_data, "cipher_text_cpp.dat");

    return 0;
}

