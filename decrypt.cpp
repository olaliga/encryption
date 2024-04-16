#include <iostream>
#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <openssl/err.h>
#include <openssl/sha.h>
#include <cstring>

void handleErrors(void);
bool verifyHMAC(const unsigned char *data, int data_len, const unsigned char *hmac, const unsigned char *key);
bool checkTokenExpiry(unsigned char *data, int token_len, int ttl);
int decryptToken(const unsigned char *token, int token_len, unsigned char *key, unsigned char *plaintext);

int main (void)
{
    /* Hardcoded key and token */
    unsigned char key[] = {
        0x32, 0x55, 0x34, 0x34, 0x73, 0x64, 0x56, 0x30, 0x4e, 0x61, 0x48, 0x44, 0x32, 0x68, 0x51, 0x30,
        0x6f, 0x46, 0x31, 0x33, 0x32, 0x48, 0x31, 0x46, 0x5f, 0x67, 0x54, 0x4f, 0x66, 0x35, 0x48, 0x4f
    };
    unsigned char token[] = {
        0x67, 0x41, 0x41, 0x41, 0x41, 0x41, 0x42, 0x6d, 0x48, 0x53, 0x38, 0x72, 0x34, 0x6a, 0x4e, 0x38,
        0x67, 0x4f, 0x37, 0x5f, 0x63, 0x72, 0x46, 0x65, 0x54, 0x55, 0x51, 0x50, 0x31, 0x45, 0x64, 0x6f,
        0x70, 0x47, 0x56, 0x72, 0x78, 0x79, 0x35, 0x42, 0x79, 0x34, 0x37, 0x62, 0x30, 0x39, 0x61, 0x4c,
        0x63, 0x6b, 0x7a, 0x69, 0x56, 0x31, 0x69, 0x63, 0x67, 0x5a, 0x2d, 0x53, 0x5a, 0x64, 0x43, 0x54,
        0x76, 0x51, 0x57, 0x52, 0x4d, 0x2d, 0x76, 0x65, 0x54, 0x57, 0x68, 0x74, 0x34, 0x4c, 0x49, 0x62,
        0x70, 0x49, 0x47, 0x37, 0x66, 0x71, 0x4d, 0x68, 0x33, 0x76, 0x30, 0x57, 0x6f, 0x5a, 0x30, 0x69,
        0x4c, 0x34, 0x31, 0x32, 0x30, 0x6e, 0x6b, 0x56, 0x2d, 0x67, 0x51, 0x61, 0x56, 0x74, 0x72, 0x51,
        0x68, 0x78, 0x66, 0x77, 0x4c, 0x6f, 0x61, 0x4d, 0x63, 0x6f, 0x45, 0x44, 0x4c, 0x68, 0x59, 0x71,
        0x4f, 0x63, 0x73, 0x63, 0x42, 0x70, 0x50, 0x54, 0x41, 0x71, 0x35, 0x43
    };

    /* Print key and token */
    std::cout << "Key: ";
    for (int i = 0; i < sizeof(key); ++i)
        printf("%02x", key[i]);
    std::cout << std::endl;

    std::cout << "Token: ";
    for (int i = 0; i < sizeof(token); ++i)
        printf("%02x", token[i]);
    std::cout << std::endl;

    /* Buffer for the decrypted text */
    unsigned char plaintext[128];

    /* Decrypt the token */
    int plaintext_len = decryptToken(token, sizeof(token), key, plaintext);

    /* Debug: Print decrypted text */
    std::cout << "Decrypted text: ";
    for (int i = 0; i < plaintext_len; ++i)
        std::cout << plaintext[i];
    std::cout << std::endl;

    return 0;
}

void handleErrors(void)
{
    ERR_print_errors_fp(stderr);
    abort();
}

bool verifyHMAC(const unsigned char *data, int data_len, const unsigned char *hmac, const unsigned char *key)
{
    unsigned char calculated_hmac[SHA256_DIGEST_LENGTH];
    HMAC(EVP_sha256(), key, 16, data, data_len, calculated_hmac, NULL);
    return (memcmp(calculated_hmac, hmac, SHA256_DIGEST_LENGTH) == 0);
}

bool checkTokenExpiry(unsigned char *data, int token_len, int ttl)
{
    time_t current_time = std::time(nullptr);
    if (token_len < 9) // Check if token is at least 9 bytes long
        return false;
    std::chrono::seconds timestamp;
    std::memcpy(&timestamp, data + 1, sizeof(timestamp));
    time_t token_time = timestamp.count();
    return (ttl == -1 || current_time <= token_time + ttl);
}

int decryptToken(const unsigned char *token, int token_len, unsigned char *key, unsigned char *plaintext)
{
    if (token_len < 41) // Check if token is at least 41 bytes long
        return 0;

    /* Extract IV and ciphertext from token */
    unsigned char iv[16];
    std::memcpy(iv, token + 9, sizeof(iv));
    const unsigned char *ciphertext = token + 25;
    int ciphertext_len = token_len - 41;

    /* Create and initialise the context */
    EVP_CIPHER_CTX *ctx;
    if (!(ctx = EVP_CIPHER_CTX_new()))
        handleErrors();

    /* Initialise the decryption operation */
    if (1 != EVP_DecryptInit_ex(ctx, EVP_aes_128_cbc(), NULL, key + 16, iv))
        handleErrors();

    /* Provide the message to be decrypted, and obtain the plaintext output */
    int plaintext_len;
    int len;
    if (1 != EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, ciphertext_len))
        handleErrors();
    plaintext_len = len;

    /* Finalise the decryption */
    if (1 != EVP_DecryptFinal_ex(ctx, plaintext + len, &len))
        handleErrors();
    plaintext_len += len;

    /* Clean up */
    EVP_CIPHER_CTX_free(ctx);

    return plaintext_len;
}
