#ifndef CRYPTO_H //헤더 가드 시작
#define CRYPTO_H //헤더 가드 정의

#include <openssl/aes.h>

void aes_encrypt(const unsigned char *key, unsigned char *data, unsigned char *encrypted_data);
void aes_decrypt(const unsigned char *key, unsigned char *encrypted_data, unsigned char *decrypted_data);

#endif // CRYPTO_H 헤더 가드 종료

