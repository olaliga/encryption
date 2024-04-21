#include <iostream>
#include <sstream>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <cstring>
#include <torch/script.h>
#include <torch/torch.h>

void handleErrors(void);
int decrypt(unsigned char *ciphertext, int ciphertext_len, unsigned char *key,
            unsigned char *iv, unsigned char *plaintext);
std::vector<char> get_the_bytes(std::string filename);

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        std::cerr << "Usage: " << argv[0] << " ciphertext_file key_file iv_file" << std::endl;
        return 1;
    }

    /* Read key from file */
    std::ifstream key_file(argv[2], std::ios::binary);
    if (!key_file) {
        std::cerr << "Failed to open key file for reading" << std::endl;
        return 1;
    }
    unsigned char key[32];
    key_file.read(reinterpret_cast<char*>(key), sizeof(key));
    key_file.close();

    /* Read iv from file */
    std::ifstream iv_file(argv[3], std::ios::binary);
    if (!iv_file) {
        std::cerr << "Failed to open iv file for reading" << std::endl;
        return 1;
    }
    unsigned char iv[16];
    iv_file.read(reinterpret_cast<char*>(iv), sizeof(iv));
    iv_file.close();

    /* Read ciphertext from file */
    std::ifstream ciphertext_file(argv[1], std::ios::binary);
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
    unsigned char *decryptedtext = new unsigned char[ciphertext_len + EVP_MAX_BLOCK_LENGTH];

    int decryptedtext_len;

    /* Decrypt the ciphertext */
    decryptedtext_len = decrypt(ciphertext, ciphertext_len, key, iv, decryptedtext);

    /* Add a NULL terminator. We are expecting printable text */
    decryptedtext[decryptedtext_len] = '\0';

    /* Load the model using decryptedtext */
    std::stringstream model_stream;
    model_stream.write(reinterpret_cast<char*>(decryptedtext), decryptedtext_len);
    torch::jit::script::Module model;
    try {
        model = torch::jit::load(model_stream);
    }
    catch (const c10::Error& e) {
        std::cerr << "Error loading the model\n";
        return 1;
    }

    // 從文件中讀取 tensor
    std::vector<char> f = get_the_bytes("test_tensor_saving.pt");
    torch::IValue x = torch::pickle_load(f);
    torch::Tensor tensor = x.toTensor();

    std::vector<torch::jit::IValue> inputs;
    inputs.push_back(tensor);
    
    // Execute the model and turn its output into a tensor.
    at::Tensor output = model.forward(inputs).toTensor();
    std::cout << output.slice(/*dim=*/1, /*start=*/0, /*end=*/5) << '\n';

    std::cout << "model loading successfully\n";

    delete[] ciphertext;
    delete[] decryptedtext;

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
    if(!(ctx = EVP_CIPHER_CTX_new())) {
        handleErrors();
    }

    /*
     * Initialise the decryption operation.
     */
    if(1 != EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv)) {
        handleErrors();
    }

    /*
     * Provide the message to be decrypted, and obtain the plaintext output.
     */
    if(1 != EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, ciphertext_len)) {
        handleErrors();
    }
    plaintext_len = len;

    /*
     * Finalise the decryption.
     */
    if(1 != EVP_DecryptFinal_ex(ctx, plaintext + len, &len)) {
        handleErrors();
    }
    plaintext_len += len;

    /* Clean up */
    EVP_CIPHER_CTX_free(ctx);

    return plaintext_len;
}

std::vector<char> get_the_bytes(std::string filename) {
    std::ifstream input(filename, std::ios::binary);
    std::vector<char> bytes(
        (std::istreambuf_iterator<char>(input)),
        (std::istreambuf_iterator<char>()));

    input.close();
    return bytes;
}
