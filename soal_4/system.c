#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>

#define MAX_HUNTERS 50
#define MAX_DUNGEONS 50

struct Hunter {
    char username[50];
    int level;
    int exp;
    int atk;
    int hp;
    int def;
    int banned;
    key_t shm_key;
};

struct Dungeon {
    char name[50];
    int min_level;
    int exp;
    int atk;
    int hp;
    int def;
    key_t shm_key;
};

struct SystemData {
    struct Hunter hunters[MAX_HUNTERS];
    int num_hunters;
    struct Dungeon dungeons[MAX_DUNGEONS];
    int num_dungeons;
    int current_notification_index;
};

key_t get_system_key() {
    return ftok("/tmp", 'S');
}

struct SystemData *system_data;
int shmid;

void cleanup(int signum) {
    shmctl(shmid, IPC_RMID, NULL);
    printf("Shared memory cleaned up.\n");
    exit(0);
}

void initialize_system() {
    key_t key = get_system_key();
    shmid = shmget(key, sizeof(struct SystemData), IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("shmget");
        exit(1);
    }
    system_data = (struct SystemData *)shmat(shmid, NULL, 0);
    if (system_data == (void *)-1) {
        perror("shmat");
        exit(1);
    }
    system_data->num_hunters = 0;
    system_data->num_dungeons = 0;
    system_data->current_notification_index = 0;
}



void display_hunters() {
    printf("Registered Hunters:\n");
    for (int i = 0; i < system_data->num_hunters; i++) {
        struct Hunter *h = &system_data->hunters[i];
        printf("Username: %s, Level: %d, EXP: %d, ATK: %d, HP: %d, DEF: %d, Banned: %s\n",
               h->username, h->level, h->exp, h->atk, h->hp, h->def, h->banned ? "Yes" : "No");
    }
}

void create_dungeon() {
    if (system_data->num_dungeons >= MAX_DUNGEONS) {
        printf("Max dungeons reached. Can't create more dungeons...\n");
        return;
    }
    struct Dungeon *dungeon = &system_data->dungeons[system_data->num_dungeons];
    sprintf(dungeon->name, "Dungeon-%d", system_data->num_dungeons + 1);
    dungeon->min_level = rand() % 5 + 1;
    dungeon->exp = rand() % 151 + 150;
    dungeon->atk = rand() % 51 + 100;
    dungeon->hp = rand() % 51 + 50;
    dungeon->def = rand() % 26 + 25;
    dungeon->shm_key = ftok("/tmp", system_data->num_dungeons + 'D');
    int dungeon_shmid = shmget(dungeon->shm_key, sizeof(struct Dungeon), IPC_CREAT | 0666);
    if (dungeon_shmid == -1) {
        perror("shmget dungeon");
        return;
    }
    struct Dungeon *dungeon_shm = (struct Dungeon *)shmat(dungeon_shmid, NULL, 0);
    *dungeon_shm = *dungeon;
    shmdt(dungeon_shm);
    system_data->num_dungeons++;
    printf("Dungeon %s created.\n", dungeon->name);
}

void display_dungeons() {
    printf("Available Dungeons:\n");
    for (int i = 0; i < system_data->num_dungeons; i++) {
        struct Dungeon *d = &system_data->dungeons[i];
        printf("Name: %s, Min Level: %d, EXP: %d, ATK: %d, HP: %d, DEF: %d, Key: %d\n",
               d->name, d->min_level, d->exp, d->atk, d->hp, d->def, d->shm_key);
    }
}

void ban_hunter(char *username) {
    for (int i = 0; i < system_data->num_hunters; i++) {
        if (strcmp(system_data->hunters[i].username, username) == 0) {
            system_data->hunters[i].banned = 1;
            printf("Hunter %s banned.\n", username);
            return;
        }
    }
    printf("Hunter %s not found.\n", username);
}

void unban_hunter(char *username) {
    for (int i = 0; i < system_data->num_hunters; i++) {
        if (strcmp(system_data->hunters[i].username, username) == 0) {
            system_data->hunters[i].banned = 0;
            printf("Hunter %s succesfuly unbanned.\n", username);
            return;
        }
    }
    printf("Hunter %s not found.\n", username);
}

void reset(char *username) {
    for (int i = 0; i < system_data->num_hunters; i++) {
        if (strcmp(system_data->hunters[i].username, username) == 0) {
            struct Hunter *h = &system_data->hunters[i];
            h->level = 1;
            h->exp = 0;
            h->atk = 10;
            h->hp = 100;
            h->def = 5;
            int hunter_shmid = shmget(h->shm_key, sizeof(struct Hunter), 0666);
            struct Hunter *hunter_shm = (struct Hunter *)shmat(hunter_shmid, NULL, 0);
            *hunter_shm = *h;
            shmdt(hunter_shm);
            printf("Hunter %s stats reset.\n", username);
            return;
        }
    }
    printf("Hunter %s not found.\n", username);
}

int main() {
    srand(time(NULL));
    signal(SIGINT, cleanup);
    initialize_system();
    char command[10], username[50];
    while (1) {
        printf("\nCommands: register, hunters, dungeon, dungeons, ban, unban, reset, exit\n");
        scanf("%s", command);
        if (strcmp(command, "register") == 0) {
            printf("Enter username: ");
            scanf("%s", username);
            register_hunter(username);
        } else if (strcmp(command, "hunters") == 0) {
            display_hunters();
        } else if (strcmp(command, "dungeon") == 0) {
            create_dungeon();
        } else if (strcmp(command, "dungeons") == 0) {
            display_dungeons();
        } else if (strcmp(command, "ban") == 0) {
            printf("Enter username: ");
            scanf("%s", username);
            ban_hunter(username);
        } else if (strcmp(command, "unban") == 0) {
            printf("Enter username: ");
            scanf("%s", username);
            unban_hunter(username);
        } else if (strcmp(command, "reset") == 0) {
            printf("Enter username: ");
            scanf("%s", username);
            reset(username);
        } else if (strcmp(command, "exit") == 0) {
            cleanup(0);
        }
    }
    shmdt(system_data);
    return 0;
}