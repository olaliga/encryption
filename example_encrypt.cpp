#include <iostream>
#include <fstream>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/err.h>
#include <cstring>

void handleErrors(void);
int encrypt(unsigned char *plaintext, int plaintext_len, unsigned char *key,
            unsigned char *iv, unsigned char *ciphertext);
int decrypt(unsigned char *ciphertext, int ciphertext_len, unsigned char *key,
            unsigned char *iv, unsigned char *plaintext);

int main (int argc, char *argv[])
{
    if (argc != 5) {
        std::cerr << "Usage: " << argv[0] << " input_file output_encrypted_file output_key_file output_iv_file" << std::endl;
        return 1;
    }

    /* Open input file */
    std::ifstream input_file(argv[1], std::ios::binary);
    if (!input_file) {
        std::cerr << "Failed to open input file" << std::endl;
        return 1;
    }

    /* Read input file content */
    input_file.seekg(0, std::ios::end);
    int plaintext_len = input_file.tellg();
    input_file.seekg(0, std::ios::beg);
    unsigned char *plaintext = new unsigned char[plaintext_len];
    input_file.read(reinterpret_cast<char*>(plaintext), plaintext_len);
    input_file.close();

    
    /* A 256 bit key */
    unsigned char key[EVP_MAX_KEY_LENGTH];
    /* A 128 bit IV */
    unsigned char iv[EVP_MAX_IV_LENGTH];

    /* Generate random key and iv */
    
    if (RAND_bytes(key, EVP_CIPHER_key_length(EVP_aes_256_cbc())) != 1 ||
        RAND_bytes(iv, EVP_CIPHER_iv_length(EVP_aes_256_cbc())) != 1) {
        handleErrors();
    }
    
    /* A 256 bit key */
    //unsigned char key[] = "01234567890123456789012345678901";

    /* A 128 bit IV */
    //unsigned char iv[] = "0123456789012345";
    /* Buffer for ciphertext */
    unsigned char ciphertext[128];

    /* Encrypt the plaintext */
    int ciphertext_len = encrypt(plaintext, plaintext_len, key, iv, ciphertext);

    /* Write encrypted content to output file */
    std::ofstream output_file(argv[2], std::ios::binary);
    if (!output_file) {
        std::cerr << "Failed to open output encrypted file" << std::endl;
        return 1;
    }
    output_file.write(reinterpret_cast<const char*>(ciphertext), ciphertext_len);
    output_file.close();

    /* Write key to output file */
    std::ofstream key_file(argv[3], std::ios::binary);
    if (!key_file) {
        std::cerr << "Failed to open output key file" << std::endl;
        return 1;
    }
    key_file.write(reinterpret_cast<const char*>(key), EVP_CIPHER_key_length(EVP_aes_256_cbc()));
    key_file.close();

    /* Write iv to output file */
    std::ofstream iv_file(argv[4], std::ios::binary);
    if (!iv_file) {
        std::cerr << "Failed to open output iv file" << std::endl;
        return 1;
    }
    iv_file.write(reinterpret_cast<const char*>(iv), EVP_CIPHER_iv_length(EVP_aes_256_cbc()));
    iv_file.close();

    delete[] plaintext;

    return 0;
}

void handleErrors(void)
{
    ERR_print_errors_fp(stderr);
    abort();
}

int encrypt(unsigned char *plaintext, int plaintext_len, unsigned char *key,
            unsigned char *iv, unsigned char *ciphertext)
{
    EVP_CIPHER_CTX *ctx;

    int len;

    int ciphertext_len;

    /* Create and initialise the context */
    if(!(ctx = EVP_CIPHER_CTX_new()))
        handleErrors();

    /*
     * Initialise the encryption operation. IMPORTANT - ensure you use a key
     * and IV size appropriate for your cipher
     * In this example we are using 256 bit AES (i.e. a 256 bit key). The
     * IV size for *most* modes is the same as the block size. For AES this
     * is 128 bits
     */
    if(1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv))
        handleErrors();

    /*
     * Provide the message to be encrypted, and obtain the encrypted output.
     * EVP_EncryptUpdate can be called multiple times if necessary
     */
    if(1 != EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, plaintext_len))
        handleErrors();
    ciphertext_len = len;

    /*
     * Finalise the encryption. Further ciphertext bytes may be written at
     * this stage.
     */
    if(1 != EVP_EncryptFinal_ex(ctx, ciphertext + len, &len))
        handleErrors();
    ciphertext_len += len;

    /* Clean up */
    EVP_CIPHER_CTX_free(ctx);

    return ciphertext_len;
}
