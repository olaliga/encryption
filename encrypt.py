from cryptography.fernet import Fernet

# 生成一個加密金鑰
key = Fernet.generate_key()
cipher_suite = Fernet(key)

# 要加密的訊息
message = b"Hello, this is a secret message!"

# 加密訊息
cipher_text = cipher_suite.encrypt(message)
print("Cipher text:", cipher_text)

# 將加密訊息儲存到檔案中
with open("cipher_text.dat", "wb") as cipher_file:
    cipher_file.write(cipher_text)

# 將金鑰儲存到檔案中，以供 C++ 使用
with open("key.key", "wb") as key_file:
    key_file.write(key)