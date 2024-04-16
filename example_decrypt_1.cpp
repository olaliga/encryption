#include <iostream>
#include <fstream>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <cstring>

void handleErrors(void);
int decrypt(unsigned char *ciphertext, int ciphertext_len, unsigned char *key,
            unsigned char *iv, unsigned char *plaintext);

int main (void)
{
    /* Read key from file */
    std::ifstream key_file("key_example.key", std::ios::binary);
    if (!key_file) {
        std::cerr << "Failed to open key file for reading" << std::endl;
        return 1;
    }
    unsigned char key[32];
    key_file.read(reinterpret_cast<char*>(key), sizeof(key));
    key_file.close();

    /* Read iv from file */
    std::ifstream iv_file("iv_example.dat", std::ios::binary);
    if (!iv_file) {
        std::cerr << "Failed to open iv file for reading" << std::endl;
        return 1;
    }
    unsigned char iv[16];
    iv_file.read(reinterpret_cast<char*>(iv), sizeof(iv));
    iv_file.close();

    /* Read ciphertext from file */
    std::ifstream ciphertext_file("ciphertext_example.dat", std::ios::binary);
    if (!ciphertext_file) {
        std::cerr << "Failed to open ciphertext file for reading" << std::endl;
        return 1;
    }
    ciphertext_file.seekg(0, std::ios::end);
    int ciphertext_len = ciphertext_file.tellg();
    ciphertext_file.seekg(0, std::ios::beg);
    unsigned char *ciphertext = new unsigned char[ciphertext_len];
    ciphertext_file.read(reinterpret_cast<char*>(ciphertext), ciphertext_len);
    ciphertext_file.close();

    /* Buffer for the decrypted text */
    unsigned char decryptedtext[128];

    int decryptedtext_len;

    /* Decrypt the ciphertext */
    decryptedtext_len = decrypt(ciphertext, ciphertext_len, key, iv,
                                decryptedtext);

    /* Add a NULL terminator. We are expecting printable text */
    decryptedtext[decryptedtext_len] = '\0';

    /* Show the decrypted text */
    std::cout << "Decrypted text is:\n";
    std::cout << decryptedtext << std::endl;

    delete[] ciphertext;

    return 0;
}


void handleErrors(void)
{
    ERR_print_errors_fp(stderr);
    abort();
}

int decrypt(unsigned char *ciphertext, int ciphertext_len, unsigned char *key,
            unsigned char *iv, unsigned char *plaintext)
{
    EVP_CIPHER_CTX *ctx;

    int len;

    int plaintext_len;

    /* Create and initialise the context */
    if(!(ctx = EVP_CIPHER_CTX_new()))
        handleErrors();

    /*
     * Initialise the decryption operation. IMPORTANT - ensure you use a key
     * and IV size appropriate for your cipher
     * In this example we are using 256 bit AES (i.e. a 256 bit key). The
     * IV size for *most* modes is the same as the block size. For AES this
     * is 128 bits
     */
    if(1 != EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv))
        handleErrors();

    /*
     * Provide the message to be decrypted, and obtain the plaintext output.
     * EVP_DecryptUpdate can be called multiple times if necessary.
     */
    if(1 != EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, ciphertext_len))
        handleErrors();
    plaintext_len = len;

    /*
     * Finalise the decryption. Further plaintext bytes may be written at
     * this stage.
     */
    if(1 != EVP_DecryptFinal_ex(ctx, plaintext + len, &len))
        handleErrors();
    plaintext_len += len;

    /* Clean up */
    EVP_CIPHER_CTX_free(ctx);

    return plaintext_len;
}
