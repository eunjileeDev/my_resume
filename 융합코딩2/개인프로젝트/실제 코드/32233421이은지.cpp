#include <conio.h>
#include <time.h>
#include <stdio.h>
#define _CRT_SECURE_NO_WARNINGS
#pragma warning(disable:4996)
#define MINUS(x, y) ((x)-(y))

#ifdef _WIN32
#include<Windows.h>
// 콘솔 종료 처리
void terminateCurses() {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_INTENSITY);  // 종료 메시지 색상
    printf("게임이 종료됩니다...\n");
}
#else //윈도우 이외의 운영체제일 때
#include <unistd.h>
void terminateCurses() {
    printf("\033[1;31m"); // 텍스트를 빨간색으로 설정
    printf("게임이 종료됩니다...\n");
    printf("\033[0m");    // 텍스트 속성을 초기화
}
#endif
// 플레이어 정보 구조체 정의
typedef struct {
    int level;
    int hp;
    int max_hp;
    int attack;
    int defense;
    char title[30]; // 칭호
} Character;

// 몬스터 정보 구조체 정의
typedef struct {
    char name[20];
    int hp;
    int attack;
    int defense;
    int collected; // 도감 등록 여부
} Monster;

// 콘솔 종료 함수
void terminateCurses();

// 몬스터 관련 함수
Monster createMonster(const char* name, int hp, int attack, int defense);
void initializeMonsterList(Monster monsters[], int* monsterCount);
Monster* getMonsterForMap(Monster* monsters, int mapLevel);

// 캐릭터 관련 함수
void initializeCharacter(Character* player);
void assignTitle(Character* player, int monstersCaught);
void levelUp(Character* player, int monstersCaught); 

// 전투 및 이벤트 처리 함수
int battle(Character* player, Monster* monster);
void handleMapEvent(Character* player, Monster* monsters, int monsterCount, int mapLevel, int* monstersCaught);  
int getUserChoice();
void displayMapInfo(int mapLevel);
void endMap(Character* player, int mapLevel, int monstersCaught);
int calculateDamage(int attackerAttack, int defenderDefense);

// 파일 관련 함수
void saveMonsterBook(Monster* monsters, int count);

// 메시지 출력 함수
void displayMainText(const char* text);
void updateStatusWindow(Character* player, int mapLevel, int monstersCaught);
void displayLevelUpMessage(Character* player);




// 메인 함수
int main() {
    Character player;
    Monster* monsters;
    int monsterCount=5; 
    int mapLevel = 1; 
    int monstersCaught = 0;

  
    // 캐릭터 및 몬스터 초기화
    initializeCharacter(&player); 
    monsters = (Monster*)malloc(sizeof(Monster) * monsterCount); 
    if (monsters == NULL) {  // 할당 실패의 경우
        printf("메모리 할당 실패\n");
        return 1;
    } 
    initializeMonsterList(monsters, &monsterCount); 

    displayMainText("초보자 헌터로서 새 삶을 시작한 것을 축하합니다!");
    updateStatusWindow(&player, mapLevel, monstersCaught);

    //대기 메시지
    displayMainText("시작하려면 아무키나 누르십시오...");
    getch();  // 키 입력 대기

    while (1) {
        handleMapEvent(&player, monsters, monsterCount, mapLevel, &monstersCaught);// 맵 이벤트 처리
        endMap(&player, mapLevel, monstersCaught); //다음 맵으로 넘어가기
        saveMonsterBook(monsters, monsterCount);
        mapLevel++;
    }
    

    // 동적 메모리 해제
    free(monsters);

    // 윈도우 종료
    terminateCurses();

    return 0;
}



// 몬스터 개별 생성 
Monster createMonster(const char* name, int hp, int attack, int defense) { 
    Monster m;
    strncpy(m.name, name, sizeof(m.name));
    m.hp = hp;
    m.attack = attack;
    m.defense = defense;
    m.collected = 0;  // 초깃값 : 수집되지 않음
    return m;
}

// 몬스터 리스트 초기화
void initializeMonsterList(Monster monsters[], int* monsterCount) {
    *monsterCount = 5; 
    monsters[0] = createMonster("슬라임", 100, 10, 5);  
    monsters[1] = createMonster("고블린", 120, 15, 10); 
    monsters[2] = createMonster("늑대인간", 140, 20, 15); 
    monsters[3] = createMonster("준보스-타이탄", 160, 30, 20); 
    monsters[4] = createMonster("보스-드래곤", 1800, 50, 30); 
}



// 캐릭터 초기화
void initializeCharacter(Character* player) {
    player->level = 1;
    player->hp = 100;  // 초기 체력
    player->max_hp = 100;
    player->attack = 10;  // 기본 공격력
    player->defense = 5;  // 기본 방어력
    player->title[0] = '\0'; //초기 칭호 비어있음
}

// 수집한 몬스터 개수 및 레벨에 따라 칭호 부여
void assignTitle(Character* player, int monstersCaught) {
    if (monstersCaught == 0) {
        strcpy(player->title, "허접하고 가난한 하수 사냥꾼");
    }
    else if (monstersCaught <= 3) {
        strcpy(player->title, "조금 경력이 생긴 중급 사냥꾼");
    }
    else if (monstersCaught <= 4) {
        strcpy(player->title, "거들먹 거릴 수 있는 고급 마스터 사냥꾼");
    }
    else {
        strcpy(player->title, "킹갓 제너럴 엠퍼러 제국 최고최강 고수 사냥꾼");
    }
}

// 특정 맵에서 등장할 몬스터 설정
Monster* getMonsterForMap(Monster* monsters, int mapLevel) {
    if (mapLevel < 1 || mapLevel > 5) {
        return NULL;  // 유효한 맵 레벨 아니면 NULL 반환
    }
    srand(time(NULL));
    int random = rand() % 100;
    if (random < 70) { //70%확률로  mapLevel에 맞는 몬스터 반환
        return &monsters[mapLevel - 1]; 
    }
    else { //30%확률로 mapLevel보다 높은 몬스터 반환
        if (mapLevel <5) {
            return &monsters[mapLevel];
        }
        else { //mapLevel 5일때는 맞는 몬스터 반환
            return &monsters[4];
        }
    }
}

// 데미지 계산 함수
int calculateDamage(int attackerAttack, int defenderDefense) {
    int damage = MINUS(attackerAttack ,defenderDefense);
    return damage > 0 ? damage : 1; // 최소 데미지를 1로 설정
}
// 전투 시스템 구현
int battle(Character* player, Monster* monster) {
    printf("전투 시작!\n");

    while (player->hp > 0 && monster->hp > 0) {
        // 플레이어의 공격
        int playerDamage = calculateDamage(player->attack, monster->defense);
        monster->hp -= playerDamage;
        printf("플레이어가 몬스터에게 %d의 데미지를 입혔습니다! (몬스터 HP: %d)\n", playerDamage, monster->hp);

        if (monster->hp <= 0) {
            printf("몬스터를 처치했습니다!\n");
            return 1;
            break;
        }

        // 몬스터의 공격
        int monsterDamage = calculateDamage(monster->attack, player->defense);
        player->hp -= monsterDamage;
        printf("몬스터가 플레이어에게 %d의 데미지를 입혔습니다! (플레이어 HP: %d)\n", monsterDamage, player->hp);

        if (player->hp <= 0) {
            printf("플레이어가 쓰러졌습니다...\n");
            break;
        }
    }
}

// 레벨업 시스템
void levelUp(Character* player, int monstersCaught) {
    player->level++;
    player->max_hp += 40;  // 체력 증가
    player->attack += 30;   // 공격력 증가
    player->defense += 20;  // 방어력 증가
    player->hp = player->max_hp;  // 체력 회복

    assignTitle(player, monstersCaught); //레벨에 따른 타이틀 획득
    displayLevelUpMessage(player);
}

// 몬스터 도감 저장 함수
void saveMonsterBook(Monster* monsters, int count) {
    FILE* file = fopen("monster_book.txt", "w");
    if (file == NULL) {
        perror("File opening failed");
        return;
    }
    for (int i = 0; i < count; i++) {
        fprintf(file, "%s %d\n", monsters[i].name, monsters[i].collected);
    }
    fclose(file);
}


// 현재 맵 정보 표시
void displayMapInfo(int mapLevel) {
    printf("현재 맵: %d\n", mapLevel);

    printf("1. 탐험하기  2. 전투하기  3. 도망 치기 4. 도감 보기 5. 상태창 보기\n");

}
// 선택지 입력 받기
int getUserChoice() {
    int choice;
    printf("원하는 선택지를 입력하세요: ");
    scanf("%d", &choice);
    return choice;
}

// 맵 이벤트 처리
void handleMapEvent(Character* player, Monster* monsters, int monsterCount, int mapLevel, int* monstersCaught) {
    Monster* monster = getMonsterForMap(monsters, mapLevel); // 맵 레벨에 맞는 몬스터 선택
   int eventCompleted = 0; //이벤트 완료 여부 플래그
    int battleAvailable = 0; // 전투 가능 여부 플래그

    while(!eventCompleted){

        displayMapInfo(mapLevel);
        int choice = getUserChoice();

        srand(time(NULL)); 

        if (choice == 2) {  // 전투 선택
            if (!battleAvailable) {  // 전투가 가능한 상태가 아닌 경우
                displayMainText("몬스터 없는 허공에 발길질 중 . . .");
            }
            else if (monster != NULL && !monster->collected) { // 몬스터가 존재하고 아직 수집되지 않은 경우
                int result = battle(player, monster);
                if (result == 1) {
                    displayMainText("몬스터를 처치했습니다!");
                    monster->collected = 1;
                    (*monstersCaught)++; // 잡은 몬스터 개수 증가

                    char message[100];
                    sprintf(message, "[%s]이 도감에 추가 되었습니다!", monster->name);
                    displayMainText(message);

                    if (strcmp(monster->name, "보스-드래곤") == 0) {
                        displayMainText("보스를 처치하셨습니다! \n폴더에 들어가 monster_book.txt파일로 수집 몬스터 목록을 확인해 보세요!");  // 보스 처치

                        terminateCurses(); //게임 종료 처리
                        exit(0);
                    }

                    levelUp(player, *monstersCaught);  // 레벨업
                }
                else {
                    displayMainText("전투에서 패배했습니다.\n 게임 오버");
                    updateStatusWindow(player, mapLevel, *monstersCaught);  // 상태창 갱신
                    terminateCurses();
                    exit(0);
                }

                // 전투 후 상태창 갱신
                if (player != NULL) {
                    updateStatusWindow(player, mapLevel, *monstersCaught);
                }

                // 몬스터 초기화 (다음 이벤트를 위해)
                monster = getMonsterForMap(monsters, mapLevel);
                battleAvailable = 0;  // 전투 후 플래그 초기화 
            }
            else {
                displayMainText("이미 몬스터를 처치했거나 수집된 상태입니다.");
            }
        }
        else if (choice == 1) { //이동 선택
            displayMainText("열심히 이동중... \n 두리번 거리는 중 . . ."); 

            int encounterChance = rand() % 100;
            if (encounterChance < 60) {  // 60% 확률로 몬스터를 만난다
                if (monster != NULL && !monster->collected) {  // 몬스터 만났을 때
                    char message[100];
                    sprintf(message, "[%s]을 만났습니다! (체력: %d, 공격력: %d, 방어력: %d)",
                        monster->name, monster->hp, monster->attack, monster->defense);
                    displayMainText(message);
                    displayMainText("전투 준비 하십시오.");
                    battleAvailable = 1;
                }
            }
            else {  // 40% 확률로 몬스터를 만나지 않는다
                displayMainText("아직도 이동 중. . . \n 아무래도 여긴 아닌 것 같다.");
            }
        }
        else if (choice == 4) {  // 도감 보기
            if (monster == NULL) {
                displayMainText("이곳에서는 더 이상 몬스터가 없습니다.");
            }
            else {
                char collectedMonsters[200] = "도감: ";
                for (int i = 0; i < monsterCount; i++) {
                    if (monsters[i].collected) {
                        strcat(collectedMonsters, monsters[i].name);
                        strcat(collectedMonsters, ", ");
                    }
                }

                if (strlen(collectedMonsters) > 6) {
                    collectedMonsters[strlen(collectedMonsters) - 2] = '\0';  // 마지막 쉼표 제거
                }

                displayMainText(collectedMonsters);
            }
        }
        else if (choice == 5) {  // 상태창 보기
            updateStatusWindow(player, mapLevel, *monstersCaught);  // 상태창 갱신
        }
        else if (choice == 3) { // 도망치기 선택
            if (battleAvailable==1) {
                displayMainText("성공적으로 도망쳤습니다! 새로운 장소로 이동합니다.");
                monster = getMonsterForMap(monsters, mapLevel); 
                battleAvailable = 0;
            }
            else {
                displayMainText("왜인지 도망치는 중. . . ");
            }
        }
        else {
            displayMainText("잘못된 입력입니다. 다시 선택해주세요.");
        }

        if (monster != NULL && monster->collected) {
            eventCompleted = 1; // 새로운 몬스터로 업데이트
        }
    }
}

// 맵 종료 후 몬스터 수집 현황 및 다음 맵 이동 
void endMap(Character* player, int mapLevel, int monstersCaught){
    displayMainText("맵을 완료했습니다.");
    printf("수집한 몬스터 수: %d\n", monstersCaught);
    printf("현재 칭호: %s\n", player->title);
    printf("다음 맵으로 이동하기: 1 \n");

    int choice = getUserChoice();
    if (choice == 1) {
        mapLevel++;
        displayMainText("다음 맵으로 이동합니다!");
    }
}
// 레벨업 축하 메시지
void displayLevelUpMessage(Character* player) {
    char msg[50];
    sprintf(msg, "축하합니다! 레벨 %d로 상승!", player->level);
    displayMainText(msg);
}

// 보스 처치 후 축하 메시지
void displayBossVictoryMessage() {
    displayMainText("보스를 처치하고 영웅 칭호를 획득했습니다!");
}



// 메인 텍스트 출력 함수
void displayMainText(const char* text) {
    printf("%s\n", text);
}
// 상태 창 갱신 함수
void updateStatusWindow(Character* player, int mapLevel, int monstersCaught) {
    printf("Status Window:\n");
    printf("Floor: %d\n", mapLevel);
    printf("LEVEL: %d\n", player->level);
    printf("HP: %d\n", player->hp);
    printf("MAX HP: %d\n", player->max_hp);
    printf("ATTACK: %d\n", player->attack);
    printf("DEFENSE: %d\n", player->defense);
    printf("Title: %s\n", player->title);
    printf("Monsters Caught: %d\n", monstersCaught);
}

