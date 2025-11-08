#include "crypto.h"
#include <openssl/aes.h>
#include <stdio.h>

// 16진수 출력 함수
void dump_hex(char *label, unsigned char buf[], int size) {
    printf("%s", label);
    for(int i = 0; i < size; i++)
        printf("%02X", buf[i]);
    printf("\n"); 
}

// AES-128 암호화 함수
void aes_encrypt(const unsigned char *key, unsigned char *data, unsigned char *encrypted_data) {
    AES_KEY encrypt_key;
    
    // AES 암호화 키 스케줄링
    if (AES_set_encrypt_key(key, 128, &encrypt_key) < 0) {
        fprintf(stderr, "암호화 키 설정 실패\n");
        return;
    }
    
    // AES 암호화
    AES_encrypt(data, encrypted_data, &encrypt_key);
    
  
}

// AES-128 복호화 함수
void aes_decrypt(const unsigned char *key, unsigned char *encrypted_data, unsigned char *decrypted_data) {
    AES_KEY decrypt_key;
    
    // AES 복호화 키 스케줄링
    if (AES_set_decrypt_key(key, 128, &decrypt_key) < 0) {
        fprintf(stderr, "복호화 키 설정 실패\n");
        return;
    }
    
    // AES 복호화
    AES_decrypt(encrypted_data, decrypted_data, &decrypt_key);

}


