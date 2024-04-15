#include <iostream>
#include <openssl/aes.h>
#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/rand.h>
#include <cstring>
#include <ctime>
#include <stdexcept>
#include <fstream>

// 使用 PKCS7 进行 unpad 操作
std::string pkcs7_unpad(const std::string& data) {
    size_t pad_size = static_cast<size_t>(data.back());
    if (pad_size > data.size()) {
        throw std::runtime_error("Invalid padding");
    }
    return data.substr(0, data.size() - pad_size);
}

// AES CBC 解密
std::string aes_cbc_decrypt(const std::string& ciphertext, const std::string& key, const std::string& iv) {
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        throw std::runtime_error("Failed to create cipher context");
    }

    if (EVP_DecryptInit_ex(ctx, EVP_aes_128_cbc(), NULL, reinterpret_cast<const unsigned char*>(key.c_str()), reinterpret_cast<const unsigned char*>(iv.c_str())) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Failed to initialize decryption");
    }

    std::string plaintext;
    int len;
    int max_len = ciphertext.size() + AES_BLOCK_SIZE; // 额外空间用于填充
    plaintext.resize(max_len);

    if (EVP_DecryptUpdate(ctx, reinterpret_cast<unsigned char*>(&plaintext[0]), &len, reinterpret_cast<const unsigned char*>(ciphertext.c_str()), ciphertext.size()) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Decryption error");
    }
    int plaintext_len = len;

    if (EVP_DecryptFinal_ex(ctx, reinterpret_cast<unsigned char*>(&plaintext[len]), &len) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Decryption finalization error");
    }
    plaintext_len += len;

    EVP_CIPHER_CTX_free(ctx);
    plaintext.resize(plaintext_len);
    return plaintext;
}

// 解密函数
std::string decrypt(const std::string& token_file, const std::string& key_file, int ttl = 0) {
    // 读取密钥文件
    std::ifstream key_ifs(key_file, std::ios::binary);
    if (!key_ifs) {
        throw std::runtime_error("Failed to open key file");
    }
    std::string base64_key((std::istreambuf_iterator<char>(key_ifs)), std::istreambuf_iterator<char>());
    key_ifs.close();

    // base64 解码密钥
    BIO* b64 = BIO_new(BIO_f_base64());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    BIO* bio = BIO_new_mem_buf(base64_key.c_str(), -1);
    bio = BIO_push(b64, bio);

    char decoded_key[1024];
    int decoded_len = BIO_read(bio, decoded_key, base64_key.size());
    if (decoded_len <= 0) {
        BIO_free_all(bio);
        throw std::runtime_error("Failed to decode key");
    }
    decoded_key[decoded_len] = '\0';
    BIO_free_all(bio);

    std::string key(decoded_key, decoded_len);

    // 读取 token 文件
    std::ifstream token_ifs(token_file, std::ios::binary);
    if (!token_ifs) {
        throw std::runtime_error("Failed to open token file");
    }
    std::string base64_token((std::istreambuf_iterator<char>(token_ifs)), std::istreambuf_iterator<char>());
    token_ifs.close();

    // base64 解码 token
    bio = BIO_new_mem_buf(base64_token.c_str(), -1);
    bio = BIO_push(b64, bio);

    char decoded_token[1024];
    decoded_len = BIO_read(bio, decoded_token, base64_token.size());
    if (decoded_len <= 0) {
        BIO_free_all(bio);
        throw std::runtime_error("Failed to decode token");
    }
    decoded_token[decoded_len] = '\0';
    BIO_free_all(bio);

    std::string token(decoded_token, decoded_len);

    // 得到当前时间戳
    std::time_t current_time = std::time(nullptr);

    // 解析 token 中的数据
    unsigned long long timestamp = 0; // 时间戳
    std::string iv; // IV
    std::string ciphertext; // 密文
    std::string hmac; // HMAC

    if (token.size() < 32) {
        throw std::runtime_error("Invalid token");
    }
    // 解析数据
    std::memcpy(&timestamp, &token[1], sizeof(timestamp));
    iv = token.substr(9, 16);
    ciphertext = token.substr(25, token.size() - 57);
    hmac = token.substr(token.size() - 32);

    // 判断密钥是否失效
    if (ttl > 0) {
        if (timestamp + ttl < current_time) {
            throw std::runtime_error("Key expired");
        }
    }

    // 计算 HMAC
    HMAC_CTX* hmac_ctx = HMAC_CTX_new();
    HMAC_Init_ex(hmac_ctx, key.c_str(), key.size(), EVP_sha256(), NULL);
    HMAC_Update(hmac_ctx, reinterpret_cast<const unsigned char*>(token.c_str()), token.size() - 32);
    unsigned char calculated_hmac[32];
    unsigned int hmac_len;
    HMAC_Final(hmac_ctx, calculated_hmac, &hmac_len);
    HMAC_CTX_free(hmac_ctx);

    // 验证 HMAC
    if (std::memcmp(calculated_hmac, hmac.c_str(), 32) != 0) {
        throw std::runtime_error("Invalid HMAC");
    }

    // AES CBC 解密
    std::string plaintext = aes_cbc_decrypt(ciphertext, key.substr(16), iv);

    // PKCS7 unpad
    std::string unpadded_plaintext = pkcs7_unpad(plaintext);

    return unpadded_plaintext;
}

int main() {
    // 假设密钥和 token 文件已经定义好了
    std::string key_file = "key.key";
    std::string token_file = "cipher_text.dat";
    int ttl = 3600; // 有效时间（秒）

    try {
        std::string decrypted_data = decrypt(token_file, key_file, ttl);
        std::cout << "Decrypted data: " << decrypted_data << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
