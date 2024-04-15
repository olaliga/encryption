from cryptography.fernet import Fernet, InvalidToken

def decrypt_with_fernet(encrypted_data_file, key_file):
    with open(key_file, "rb") as key_f:
        key = key_f.read().strip()

    with open(encrypted_data_file, "rb") as encrypted_data_f:
        encrypted_data = encrypted_data_f.read().strip()

    f = Fernet(key)
    try:
        decrypted_data = f.decrypt(encrypted_data)
        return decrypted_data.decode("utf-8")
    except InvalidToken:
        print("Error: Invalid token. The ciphertext may have been tampered with or the key is incorrect.")
        return None

def main():
    decrypted_data = decrypt_with_fernet("cipher_text_cpp.dat", "key_cpp.key")
    if decrypted_data is not None:
        print("Decrypted data:", decrypted_data)

if __name__ == "__main__":
    main()


