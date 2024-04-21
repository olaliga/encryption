#include <iostream>
#include <fstream>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/err.h>
#include <cstring>
#include <torch/script.h>
#include <torch/torch.h>

void handleErrors(void);
int encrypt(unsigned char *plaintext, int plaintext_len, unsigned char *key,
            unsigned char *iv, unsigned char *ciphertext);

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

    // check model load success
    torch::jit::script::Module model;
    try {
        // Deserialize the ScriptModule from a file using torch::jit::load().
        model = torch::jit::load(input_file);
    }
    catch (const c10::Error& e) {
        std::cerr << "error loading the model\n";
        return 1;
    }

    // Create a vector of inputs.
    std::vector<torch::jit::IValue> inputs;
    inputs.push_back(torch::rand({2, 3, 224, 224}));

    // Execute the model and turn its output into a tensor.
    at::Tensor output = model.forward(inputs).toTensor();
    std::cout << output.slice(/*dim=*/1, /*start=*/0, /*end=*/5) << '\n';
    std::cout << "model loading ok\n";

    // saving example tensor
    torch::save(inputs, "example_tensor.pt");

    /* Read input file content */
    input_file.seekg(0, std::ios::end);
    int plaintext_len = input_file.tellg();
    input_file.seekg(0, std::ios::beg);
    unsigned char *plaintext = new unsigned char[plaintext_len];
    input_file.read(reinterpret_cast<char*>(plaintext), plaintext_len);
    input_file.close();

    std::cout << "Read file content loading ok\n";

    /* A 256 bit key */
    unsigned char key[EVP_MAX_KEY_LENGTH];
    /* A 128 bit IV */
    unsigned char iv[EVP_MAX_IV_LENGTH];

    /* Generate random key and iv */
    if (RAND_bytes(key, EVP_CIPHER_key_length(EVP_aes_256_cbc())) != 1 ||
        RAND_bytes(iv, EVP_CIPHER_iv_length(EVP_aes_256_cbc())) != 1) {
        handleErrors();
    }

    std::cout << "key and iv generate ok\n";

    /* Buffer for ciphertext 
    Dynamic memory allocation issue: In the main function, if you used unsigned char ciphertext[128]; 
    to define ciphertext, this size might not be sufficient, leading to encrypted data exceeding this range. 
    Consider dynamically allocating a large enough memory space to store the encrypted data and 
    releasing it in a timely manner when it is no longer needed
    */
    unsigned char *ciphertext = new unsigned char[plaintext_len + EVP_MAX_BLOCK_LENGTH];

    /* Encrypt the plaintext */
    int ciphertext_len = encrypt(plaintext, plaintext_len, key, iv, ciphertext);
    std::cout << "ciphertext ok\n";

    /* Write encrypted content to output file */
    std::ofstream output_file(argv[2], std::ios::binary);
    if (!output_file) {
        std::cerr << "Failed to open output encrypted file" << std::endl;
        return 1;
    }
    output_file.write(reinterpret_cast<const char*>(ciphertext), ciphertext_len);
    output_file.close();
    std::cout << "encrypt model file ok\n";

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
    delete[] ciphertext;

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
    EVP_CIPHER_CTX* ctx;

    int len;
    int ciphertext_len = 0;

    /* Create and initialise the context */
    if (!(ctx = EVP_CIPHER_CTX_new()))
        handleErrors();

    std::cout << "Create and initialise the context ok\n";

    /*
     * Initialise the encryption operation. IMPORTANT - ensure you use a key
     * and IV size appropriate for your cipher
     * In this example we are using 256 bit AES (i.e. a 256 bit key). The
     * IV size for *most* modes is the same as the block size. For AES this
     * is 128 bits
     */
    if (1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv))
        handleErrors();

    std::cout << "Initialise the encryption operation ok\n";

    /*
     * Provide the message to be encrypted, and obtain the encrypted output.
     * EVP_EncryptUpdate can be called multiple times if necessary
     */
    int outlen;
    int remaining = plaintext_len;
    int block_size = EVP_CIPHER_CTX_block_size(ctx);
    unsigned char *p = plaintext;

    while (remaining > 0)
    {
        // 計算此次加密的數據大小
        int to_encrypt = (remaining > block_size) ? block_size : remaining;

        /*
         * Provide the message to be encrypted, and obtain the encrypted output.
         * EVP_EncryptUpdate can be called multiple times if necessary
         */
        if (1 != EVP_EncryptUpdate(ctx, ciphertext + ciphertext_len, &outlen, p, to_encrypt))
            handleErrors();

        ciphertext_len += outlen;
        p += to_encrypt;         // 更新待加密數據指針
        remaining -= to_encrypt; // 更新剩餘待加密數據長度
    }

    std::cout << "Obtain the encrypted output ok\n";

    /*
     * Finalise the encryption. Further ciphertext bytes may be written at
     * this stage.
     */
    if (1 != EVP_EncryptFinal_ex(ctx, ciphertext + ciphertext_len, &len))
        handleErrors();

    ciphertext_len += len;

    std::cout << "Finalise the encryption ok\n";

    /* Clean up */
    EVP_CIPHER_CTX_free(ctx);
    std::cout << "Clean up ok\n";

    return ciphertext_len;
}
