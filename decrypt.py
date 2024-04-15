from cryptography.fernet import Fernet

# 讀取金鑰
with open("key_cpp.key", "rb") as key_file:
    key = key_file.read()

# 讀取加密後的訊息
with open("cipher_text_cpp.dat", "rb") as cipher_file:
    cipher_text = cipher_file.read()

# 使用金鑰建立 Fernet 實例
cipher_suite = Fernet(key)

# 解密訊息
plain_text = cipher_suite.decrypt(cipher_text)
print("Decrypted message:", plain_text.decode())
