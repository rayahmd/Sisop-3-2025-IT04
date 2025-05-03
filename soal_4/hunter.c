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
struct Hunter *cur_hunter;
int shmid, hunter_shmid;
pthread_t notification_thread;
int notification_running = 0;

void *notification_loop(void *arg) {
    while (notification_running) {
        system("clear");
        printf("Dungeon Notifications for %s:\n", cur_hunter->username);
        if (system_data->num_dungeons == 0) {
            printf("No dungeons available.\n");
        } else {
            int index = system_data->current_notification_index % system_data->num_dungeons;
            struct Dungeon *d = &system_data->dungeons[index];
            if (d->min_level <= cur_hunter->level) {
                printf("Name: %s, Min Level: %d, EXP: %d, ATK: %d, HP: %d, DEF: %d\n",
                       d->name, d->min_level, d->exp, d->atk, d->hp, d->def);
            }
            system_data->current_notification_index++;
        }
        sleep(3);
    }
    return NULL;
}

void register_hunter(char *username) {
    key_t key = get_system_key();
    shmid = shmget(key, sizeof(struct SystemData), 0666);
    if (shmid == -1) {
        perror("shmget");
        exit(1);
    }
    system_data = (struct SystemData *)shmat(shmid, NULL, 0);
    if (system_data->num_hunters >= MAX_HUNTERS) {
        printf("Max hunters reached. Can't register again:(\n");
        return;
    }
    for (int i = 0; i < system_data->num_hunters; i++) {
        if (strcmp(system_data->hunters[i].username, username) == 0) {
            printf("Username already exists.\n");
            return;
        }
    }
    struct Hunter *hunter = &system_data->hunters[system_data->num_hunters];
    strcpy(hunter->username, username);
    hunter->level = 1;
    hunter->exp = 0;
    hunter->atk = 10;
    hunter->hp = 100;
    hunter->def = 5;
    hunter->banned = 0;
    hunter->shm_key = ftok("/tmp", system_data->num_hunters + 'H');
    int hunter_shmid = shmget(hunter->shm_key, sizeof(struct Hunter), IPC_CREAT | 0666);
    if (hunter_shmid == -1) {
        perror("shmget hunter");
        return;
    }
    struct Hunter *hunter_shm = (struct Hunter *)shmat(hunter_shmid, NULL, 0);
    *hunter_shm = *hunter;
    shmdt(hunter_shm);
    system_data->num_hunters++;
    printf("Hunter %s registered.\n", username);
}

int login_hunter(char *username) {
    key_t key = get_system_key();
    shmid = shmget(key, sizeof(struct SystemData), 0666);
    if (shmid == -1) {
        perror("shmget");
        exit(1);
    }
    system_data = (struct SystemData *)shmat(shmid, NULL, 0);
    for (int i = 0; i < system_data->num_hunters; i++) {
        if (strcmp(system_data->hunters[i].username, username) == 0) {
            cur_hunter = &system_data->hunters[i];
            hunter_shmid = shmget(cur_hunter->shm_key, sizeof(struct Hunter), 0666);
            if (hunter_shmid == -1) {
                perror("shmget hunter");
                exit(1);
            }
            cur_hunter = (struct Hunter *)shmat(hunter_shmid, NULL, 0);
            printf("Logged in as %s.\n", username);
            return;
        }
    }
    printf("Hunter %s not found.\n", username);
    shmdt(system_data);
    return 0;
}

void display_ur_dungeons() {
    printf("Available Dungeons for Hunter %s level: %d\n", cur_hunter->username, cur_hunter->level);
    for (int i = 0; i < system_data->num_dungeons; i++) {
        struct Dungeon *d = &system_data->dungeons[i];
        if (d->min_level <= cur_hunter->level) {
            printf("Name: %s, Min Level: %d, EXP: %d, ATK: %d, HP: %d, DEF: %d\n",
                   d->name, d->min_level, d->exp, d->atk, d->hp, d->def);
        }
    }
}

void battle(char *opponent_username) {
    if (cur_hunter->banned) {
        printf("You are banned.\n");
        return;
    }
    for (int i = 0; i < system_data->num_hunters; i++) {
        if (strcmp(system_data->hunters[i].username, opponent_username) == 0) {
            struct Hunter *opponent = &system_data->hunters[i];
            if (opponent->banned) {
                printf("Opponent is banned.\n");
                return;
            }
            int my_stats = cur_hunter->atk + cur_hunter->hp + cur_hunter->def;
            int opp_stats = opponent->atk + opponent->hp + opponent->def;
            if (my_stats > opp_stats) {
                cur_hunter->atk += opponent->atk;
                cur_hunter->hp += opponent->hp;
                cur_hunter->def += opponent->def;
                int opp_shmid = shmget(opponent->shm_key, sizeof(struct Hunter), 0666);
                shmctl(opp_shmid, IPC_RMID, NULL);
                for (int j = i; j < system_data->num_hunters - 1; j++) {
                    system_data->hunters[j] = system_data->hunters[j + 1];
                }
                system_data->num_hunters--;
                printf("You defeated %s!\n", opponent_username);
            } else {
                opponent->atk += cur_hunter->atk;
                opponent->hp += cur_hunter->hp;
                opponent->def += cur_hunter->def;
                shmctl(hunter_shmid, IPC_RMID, NULL);
                for (int j = 0; j < system_data->num_hunters; j++) {
                    if (strcmp(system_data->hunters[j].username, cur_hunter->username) == 0) {
                        for (int k = j; k < system_data->num_hunters - 1; k++) {
                            system_data->hunters[k] = system_data->hunters[k + 1];
                        }
                        system_data->num_hunters--;
                        break;
                    }
                }
                printf("You were defeated by %s!\n", opponent_username);
                shmdt(cur_hunter);
                shmdt(system_data);
                exit(0);
            }
            return;
        }
    }
    printf("Opponent %s not found.\n", opponent_username);
}

void toggle_notif() {
    if (notification_running) {
        notification_running = 0;
        pthread_join(notification_thread, NULL);
        printf("Notifications stopped.\n");
    } else {
        notification_running = 1;
        pthread_create(&notification_thread, NULL, notification_loop, NULL);
        printf("Notifications started.\n");
    }
}

void hunter_menu() {
    int choice;
    char opponent[50];
    while (1) {
        printf("\n=== %s's MENU ===\n", cur_hunter->username);
        printf("1. List Dungeon\n");
        printf("2. Raid\n");
        printf("3. Battle\n");
        printf("4. Toggle Notification\n");
        printf("5. Exit\n");
        printf("Choice: ");
        scanf("%d", &choice);
        getchar(); // Clear newline from input buffer
        if (choice == 1) {
            if (cur_hunter->banned) {
                printf("You are banned from raiding.\n");
            } else {
                display_available_dungeons();
            }
        } else if (choice == 2) {
            if (cur_hunter->banned) {
                printf("You are banned from raiding.\n");
            } else {
                raid_dungeon();
            }
        } else if (choice == 3) {
            printf("Enter opponent username: ");
            scanf("%s", opponent);
            battle_hunter(opponent);
        } else if (choice == 4) {
            toggle_notifications();
        } else if (choice == 5) {
            shmdt(cur_hunter);
            shmdt(system_data);
            break;
        } else {
            printf("Invalid choice.\n");
        }
    }
}


int main() {
    int choice;
    char username[50];
    while (1) {
        printf("\n=== HUNTER MENU ===\n");
        printf("= 1. Register\n");
        printf("= 2. Login\n");
        printf("= 3. Exit\n");
        printf("======================");
        scanf("%d", &choice);
        getchar(); 
        switch(choice) {
            case 1: {
            printf("username: ");
            scanf("%s", username);
            register_hunter(username);
        } case 2:{
            printf("username: ");
            scanf("%s", username);
            if (login_hunter(username)) {
                hunter_menu();
            }
        } case 3: {
            printf("Exiting...\n");
            break;
        } default: {
            printf("Please choose the right option!\n");
        }
    }
    return 0;
}