#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define MAX_WEAPONS 10
typedef struct {
    char name[50];
    int price;
    int damage;
    char passive[100];
    int passive_active;
} Weapon;
extern Weapon weapons[MAX_WEAPONS];
extern void get_weapon_list(char *buffer);
extern int buy_weapon(int weapon_id, int *gold, char *inventory, int *inventory_size);

#define PORT 8080
#define MAX_CLIENTS 10
#define MAX_INVENTORY 20

typedef struct {
    int gold;
    char equipped_weapon[50];
    int base_damage;
    int kills;
    char passive[100];
    char inventory[500];
    int inventory_size;
    int in_battle;
    int enemy_hp;
    int enemy_max_hp;
    int attack_count;
    int flambeu_burn_stack;
    int verdant_splitbow_fail_count;
    int codex_carmine_fail_count;
    int codex_carmine_bonus_dmg;
    int lunar_fang_fail_count;
    int reaper_triggered;
    int bloodshatter_triggered;
} Player;

Player players[MAX_CLIENTS];
pthread_mutex_t player_mutex = PTHREAD_MUTEX_INITIALIZER;

void initialize_player(int client_id) {
    players[client_id].gold = 500;
    strcpy(players[client_id].equipped_weapon, "Fists");
    players[client_id].base_damage = 5;
    players[client_id].kills = 0;
    strcpy(players[client_id].passive, "");
    strcpy(players[client_id].inventory, "Fists|");
    players[client_id].inventory_size = 1;
    players[client_id].in_battle = 0;
    players[client_id].enemy_hp = 0;
    players[client_id].enemy_max_hp = 0;
    players[client_id].attack_count = 0;
    players[client_id].flambeu_burn_stack = 0;
    players[client_id].verdant_splitbow_fail_count = 0;
    players[client_id].codex_carmine_fail_count = 0;
    players[client_id].codex_carmine_bonus_dmg = 0;
    players[client_id].lunar_fang_fail_count = 0;
    players[client_id].reaper_triggered = 0;
    players[client_id].bloodshatter_triggered = 0;
}

void send_response(int sock, char *response) {
    size_t len = strlen(response);
    write(sock, response, len);
}

const char *pink_foreground = "\033[38;5;218m";
const char *reset = "\033[0m";
const char *purple_foreground = "\033[38;5;207m";
const char *red_foreground = "\033[38;5;196m";
const char *yellow_foreground = "\033[38;5;226m";
const char *passive_foreground = "\033[38;5;141m";
const char *failed_foreground = "\033[38;5;147m";
const char *orange_foreground = "\033[38;5;214m";
const char *magenta_foreground = "\033[38;5;212m";
const char *light_foreground = "\033[38;5;222m";

void get_player_stats(int client_id, char *buffer) {
    Player *p = &players[client_id];
    snprintf(buffer, 2048,
        "\n%s(𐑘   /)\n ˶ᵔ ᵕ ᵔ˶           P L A Y E R   S T A T S\n꒰%s"
        "%sGold: %d%s" "%s | %s" "%sEquipped Weapon: %s%s" "%s | %s" "%sBase Damage: %d%s" "%s | %s" "%sKills: %d%s" "%s ꒱%s",
        magenta_foreground, reset, yellow_foreground, p->gold, reset, magenta_foreground, reset, light_foreground, p->equipped_weapon, reset, magenta_foreground,
        reset, orange_foreground, p->base_damage, reset, magenta_foreground, reset, red_foreground, p->kills, reset, magenta_foreground, reset);
    if (strlen(p->passive) > 0) {
        char temp[200];
        snprintf(temp, sizeof(temp), "%s | %s" "%sPassive: %.100s%s", magenta_foreground, reset, passive_foreground, p->passive, reset);
        strncat(buffer, temp, 2048 - strlen(buffer) - 1);
    }
    strncat(buffer, "\n", 2048 - strlen(buffer) - 1);
}

void get_inventory(int client_id, char *buffer) {
    Player *p = &players[client_id];
    snprintf(buffer, 2048 - strlen(buffer) - 1, "\n%s(𐑘   /)\n ˶ᵔ ᵕ ᵔ˶           I N V E N T O R Y%s\n", orange_foreground, reset);
    char inventory_copy[500];
    strncpy(inventory_copy, p->inventory, sizeof(inventory_copy) - 1);
    inventory_copy[sizeof(inventory_copy) - 1] = '\0';
    char *token = strtok(inventory_copy, "|");
    int idx = 0;
    while (token != NULL) {
        char temp[200];
        int weapon_idx = -1;
        for (int i = 0; i < MAX_WEAPONS; i++) {
            if (strcmp(token, weapons[i].name) == 0) {
                weapon_idx = i;
                break;
            }
        }
        if (weapon_idx >= 0 && weapons[weapon_idx].passive_active) {
            snprintf(temp, sizeof(temp), "%s꒰%d ꒱ %.50s%s %s(Passive: %.100s)%s%s\n",
                     light_foreground, idx, token, reset, passive_foreground, weapons[weapon_idx].passive,
                     strcmp(token, p->equipped_weapon) == 0 ? " (EQUIPPED)" : "", reset);
        } else {
            snprintf(temp, sizeof(temp), "%s꒰%d ꒱ %.50s%s%s\n", light_foreground, idx, token,
                     strcmp(token, p->equipped_weapon) == 0 ? " (EQUIPPED)" : "", reset);
        }
        strncat(buffer, temp, 2048 - strlen(buffer) - 1);
        token = strtok(NULL, "|");
        idx++;
    }
}

int equip_weapon(int client_id, int weapon_id) {
    Player *p = &players[client_id];
    char inventory_copy[500];
    strncpy(inventory_copy, p->inventory, sizeof(inventory_copy) - 1);
    inventory_copy[sizeof(inventory_copy) - 1] = '\0';

    int idx = 0;
    char *start = inventory_copy;
    char *end;
    while (*start && idx <= weapon_id) {
        end = strchr(start, '|');
        if (!end) break;
        *end = '\0';
        while (*start == ' ') start++;
        char *trim_end = end - 1;
        while (trim_end > start && (*trim_end == ' ' || *trim_end == '\n')) *trim_end-- = '\0';
        if (idx == weapon_id && strlen(start) > 0) {
            strcpy(p->equipped_weapon, start);
            for (int i = 0; i < MAX_WEAPONS; i++) {
                if (strcmp(start, weapons[i].name) == 0) {
                    p->base_damage = weapons[i].damage;
                    strcpy(p->passive, weapons[i].passive);
                    return 1;
                }
            }
            if (strcmp(start, "Fists") == 0) {
                p->base_damage = 5;
                strcpy(p->passive, "");
                return 1;
            }
            p->base_damage = 5;
            strcpy(p->passive, "");
            return 1;
        }
        start = end + 1;
        idx++;
    }
    return 0;
}

void start_battle(int client_id) {
    Player *p = &players[client_id];
    p->in_battle = 1;
    const int min_hp = 50;
    const int max_hp = 200;
    p->enemy_max_hp = min_hp + (rand() % (max_hp - min_hp + 1));
    p->enemy_hp = p->enemy_max_hp;
    p->attack_count = 0;
    p->flambeu_burn_stack = 0;
    p->verdant_splitbow_fail_count = 0;
    p->codex_carmine_fail_count = 0;
    p->codex_carmine_bonus_dmg = 0;
    p->lunar_fang_fail_count = 0;
    p->reaper_triggered = 0;
    p->bloodshatter_triggered = 0;
}

void display_enemy_status(int client_id, char *buffer, int is_new_battle) {
    Player *p = &players[client_id];
    if (p->enemy_max_hp <= 0 || p->enemy_hp < 0) {
        snprintf(buffer, 2048, "Error: Invalid enemy HP!\n");
        return;
    }
    
    const int total_bars = 50;
    int filled_bars = (p->enemy_hp * total_bars) / p->enemy_max_hp;
    if (filled_bars < 0) filled_bars = 0;
    if (filled_bars > total_bars) filled_bars = total_bars;

    float hp_percent = (float)p->enemy_hp / p->enemy_max_hp * 100;
    const char *hp_color;
    if (hp_percent >= 50) {
        hp_color = "\033[38;5;218m";
    } else if (hp_percent >= 20) {
        hp_color = "\033[38;5;210m";
    } else {
        hp_color = "\033[38;5;203m";
    }

    char health_bar[480] = "꒰";
    for (int i = 0; i < total_bars; i++) {
        strncat(health_bar, i < filled_bars ? "█" : " ", 480 - strlen(health_bar) - 1);
    }
    strncat(health_bar, " ꒱", 480 - strlen(health_bar) - 1);

    if (is_new_battle) {
        snprintf(buffer, 2048,
            "\n%s(𐑘   /)        B A T T L E   S T A R T E D %s\n"
            "%s ˶ᵔ ᵕ ᵔ˶          fierce enemy appears!\n"
            "%s%s%s%s %s%d/%d%s"
            "%s HP%s\n"
            "%s  ୨♡୧ ... Type 'attack' to strike or 'exit' to flee!%s\n",
            pink_foreground, reset,
            pink_foreground, reset,
            hp_color, health_bar, reset, pink_foreground,
            p->enemy_hp, p->enemy_max_hp, reset,
            pink_foreground, reset,
            pink_foreground, reset);        
    } else {
        snprintf(buffer, 2048,
            "\n%s(𐑘   /)          E N E M Y   S T A T U S %s\n"
            "%s ˶ᵔ ᵕ ᵔ˶             fight the enemy!\n"
            "%s%s%s%s %s%d/%d%s"
            "%s HP%s\n"
            "%s  ୨♡୧ ... Type 'attack' to strike or 'exit' to flee!%s\n",
            pink_foreground, reset,
            pink_foreground, reset,
            hp_color, health_bar, reset, pink_foreground,
            p->enemy_hp, p->enemy_max_hp, reset,
            pink_foreground, reset,
            pink_foreground, reset);
    }
}

void attack_enemy(int client_id, char *buffer) {
    Player *p = &players[client_id];
    int damage = p->base_damage + (rand() % 10);
    int is_crit = rand() % 100 < 20;
    int is_insta_kill = 0;
    int bonus_dmg = 0;
    p->attack_count++;

    size_t offset = 0;
    buffer[0] = '\0';

    if (strcmp(p->equipped_weapon, "Flambeu") == 0) {
        p->flambeu_burn_stack = (p->flambeu_burn_stack < 30) ? p->flambeu_burn_stack + 5 : 30;
        bonus_dmg = (p->base_damage * p->flambeu_burn_stack) / 100;
        sprintf(buffer, "\n%s/ ❦ . . ꒰Burn effect deals %d true damage! ꒱%s\n",
            passive_foreground, bonus_dmg, reset);
    } else if (strcmp(p->equipped_weapon, "Verdant Splitbow") == 0) {
        int bloom_chance = 30 + (p->verdant_splitbow_fail_count * 10);
        if (rand() % 100 < bloom_chance) {
            damage *= 3;
            p->verdant_splitbow_fail_count = 0;
            sprintf(buffer, "\n%s/ ❦ . . ꒰Verdant Splitbow blooms into 3 arrows! Deals %d damage! ꒱%s\n",
                passive_foreground, damage, reset);
        } else {
            p->verdant_splitbow_fail_count++;
            sprintf(buffer, "\n%s/ ❦ . . ꒰Verdant Splitbow fails to bloom. ꒱%s\n",
                failed_foreground, reset);
        }
    } else if (strcmp(p->equipped_weapon, "Detonark") == 0 && p->attack_count % 3 == 0) {
        bonus_dmg = (p->enemy_max_hp * 15) / 100;
        sprintf(buffer, "\n%s/ ❦ . . ꒰Detonark explodes for %d damage! ꒱%s\n",
            passive_foreground, bonus_dmg, reset);
    } else if (strcmp(p->equipped_weapon, "Codex Carmine") == 0) {
        int drain_chance = (p->codex_carmine_fail_count >= 2) ? 40 : 30;
        if (rand() % 100 < drain_chance) {
            int drained_hp = (p->enemy_hp * 10) / 100;
            p->codex_carmine_bonus_dmg = drained_hp;
            p->codex_carmine_fail_count = 0;
            sprintf(buffer, "\n%s/ ❦ . . ꒰Codex Carmine drains %d HP for next strike! ꒱%s\n",
                passive_foreground, drained_hp, reset);
        } 
    } else if (strcmp(p->equipped_weapon, "Lunar Fang") == 0) {
        int crit_chance = 15 + (p->lunar_fang_fail_count * 5);
        crit_chance = (crit_chance > 30) ? 30 : crit_chance;
        if (rand() % 100 < crit_chance) {
            is_crit = 1;
            p->lunar_fang_fail_count = 0;
            sprintf(buffer, "\n%s/ ❦ . . ꒰Lunar Fang unleashes wolf spirit! Critical hit! ꒱%s\n",
                passive_foreground, reset);
        }
    } else if (strcmp(p->equipped_weapon, "Culling Spear") == 0 && p->enemy_hp <= (p->enemy_max_hp * 20) / 100) {
        is_insta_kill = 1;
        sprintf(buffer, "\n%s/ ❦ . . ꒰Culling Spear executes the enemy! ꒱%s\n",
            passive_foreground, reset);
    } else if (strcmp(p->equipped_weapon, "Reaper's Covenant") == 0 && !p->reaper_triggered) {
        if (rand() % 100 < 15) {
            bonus_dmg = p->enemy_hp / 2;
            p->reaper_triggered = 1;
            sprintf(buffer, "\n%s/ ❦ . . ꒰Reaper's Covenant reaps %d HP! ꒱%s\n",
                passive_foreground, bonus_dmg, reset);
        }
    } else if (strcmp(p->equipped_weapon, "Bloodshatter Blade") == 0 && !p->bloodshatter_triggered && p->enemy_max_hp > 120) {
        bonus_dmg = p->enemy_max_hp / 2;
        p->bloodshatter_triggered = 1;
        sprintf(buffer, "\n%s/ ❦ . . ꒰Bloodshatter Blade consumes %d HP! ꒱%s\n",
            passive_foreground, bonus_dmg, reset);
    } else if (strcmp(p->equipped_weapon, "Trispear of Ascension") == 0) {
        if (rand() % 100 < 20) {
            is_insta_kill = 1;
            sprintf(buffer, "\n%s/ ❦ . . ꒰Trispear of Ascension instantly kills the enemy! ꒱%s\n",
                passive_foreground, reset);
        } else {
            bonus_dmg = (p->enemy_max_hp * 25) / 100;
            sprintf(buffer, "\n%s/ ❦ . . ꒰Trispear of Ascension deals %d lightning damage! ꒱%s\n",
                failed_foreground, bonus_dmg, reset);
        }
    }

    if (is_insta_kill) {
        p->enemy_hp = 0;
    } else {
        if (is_crit) {
            damage *= 2;
            char temp[200];
            sprintf(temp, "\n%s .˳⁺⁎˚ ꒰ఎ CRITICAL HIT! ໒ ꒱ ˚⁎⁺˳ . %s\n%s/ ❦ . . ꒰You dealt %d damage! ꒱%s\n",
                red_foreground, reset, purple_foreground, damage, reset);
            strcat(buffer, temp);
        } else if (strcmp(p->equipped_weapon, "Flambeu") != 0 && strcmp(p->equipped_weapon, "Verdant Splitbow") != 0 &&
                   strcmp(p->equipped_weapon, "Detonark") != 0 && strcmp(p->equipped_weapon, "Codex Carmine") != 0 &&
                   strcmp(p->equipped_weapon, "Reaper's Covenant") != 0 && strcmp(p->equipped_weapon, "Bloodshatter Blade") != 0 &&
                   strcmp(p->equipped_weapon, "Trispear of Ascension") != 0) {
            char temp[200];
            sprintf(temp, "\n%s/ ❦ . . ꒰You dealt %d damage! ꒱%s\n", purple_foreground, damage, reset);
            strcat(buffer, temp);
        }
        p->enemy_hp -= (damage + bonus_dmg + p->codex_carmine_bonus_dmg);
        p->codex_carmine_bonus_dmg = 0;
    }

    if (p->enemy_hp <= 0) {
        p->kills++;
        int reward = 50 + (rand() % 101);
        p->gold += reward;
    
        char temp[256];
        sprintf(temp, 
            "\n%s          R E W A R D\n        ︶︶︶୨♡୧︶︶︶\n  ୨♡୧ ... You earned %d gold!%s\n",
            yellow_foreground, reward, reset);
        strcat(buffer, temp);
    
        start_battle(client_id);
    
        char temp2[2048];
        display_enemy_status(client_id, temp2, 1);
    
        strcat(buffer, "\n");
        strcat(buffer, red_foreground);
        strcat(buffer, "/ ❦ . . ꒰NEW ENEMY HAS APPEARED! ꒱");
        strcat(buffer, reset);
    
        strcat(buffer, temp2);
    }
     else {
        char temp[2048];
        display_enemy_status(client_id, temp, 0);
        strcat(buffer, temp);
    }
}

int next_client_id = 0;
pthread_mutex_t id_mutex = PTHREAD_MUTEX_INITIALIZER;

void *handle_client(void *arg) {
    int client_sock = *(int *)arg;
    pthread_mutex_lock(&id_mutex);
    int client_id = next_client_id++;
    if (client_id >= MAX_CLIENTS) {
        next_client_id = 0;
        client_id = next_client_id++;
    }
    pthread_mutex_unlock(&id_mutex);
    
    pthread_mutex_lock(&player_mutex);
    initialize_player(client_id);
    pthread_mutex_unlock(&player_mutex);

    char buffer[1024];
    while (1) {
        int n = read(client_sock, buffer, sizeof(buffer) - 1);
        if (n <= 0) {
            break;
        }
        buffer[n] = '\0';

        pthread_mutex_lock(&player_mutex);
        char response[2048] = "";
        if (players[client_id].in_battle) {
            if (strcmp(buffer, "attack") == 0) {
                attack_enemy(client_id, response);
            } else if (strcmp(buffer, "exit") == 0) {
                players[client_id].in_battle = 0;
                strcpy(response, "You left the battle.");
            } else {
                strcpy(response, "Invalid command. Type 'attack' or 'exit'.\n");
            }
        } else {
            int option = atoi(buffer);
            switch (option) {
                case 1:
                    get_player_stats(client_id, response);
                    break;
                case 2:
                    get_weapon_list(response);
                    send_response(client_sock, response);
                    n = read(client_sock, buffer, sizeof(buffer) - 1);
                    if (n <= 0) break;
                    buffer[n] = '\0';
                    int weapon_id = atoi(buffer);
                    if (weapon_id == 0) {
                        strcpy(response, "Purchase cancelled.\n");
                    } else if (buy_weapon(weapon_id, &players[client_id].gold, players[client_id].inventory,
                                          &players[client_id].inventory_size)) {
                        strcpy(response, "Weapon purchased successfully!\n");
                    } else {
                        strcpy(response, "Not enough gold or invalid weapon.\n");
                    }
                    break;
                case 3:
                    get_inventory(client_id, response);
                    send_response(client_sock, response);
                    n = read(client_sock, buffer, sizeof(buffer) - 1);
                    if (n <= 0) break;
                    buffer[n] = '\0';
                    int equip_id = atoi(buffer);
                    if (equip_weapon(client_id, equip_id)) {
                        strcpy(response, "Weapon equipped successfully!\n");
                    } else {
                        strcpy(response, "Invalid weapon ID.\n");
                    }
                    break;
                case 4:
                    start_battle(client_id);
                    display_enemy_status(client_id, response, 1);
                    break;
                case 5:
                    strcpy(response, "Goodbye!\n");
                    send_response(client_sock, response);
                    close(client_sock);
                    pthread_mutex_unlock(&player_mutex);
                    return NULL;
                default:
                    strcpy(response, "Invalid option. Please try again.\n");
            }
        }
        send_response(client_sock, response);
        pthread_mutex_unlock(&player_mutex);
    }
    close(client_sock);
    return NULL;
}

int main() {
    srand(time(NULL));
    int server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr));
    listen(server_sock, MAX_CLIENTS);

    printf("/ ❦ . . ꒰Server running on port %d... ꒱\n", PORT);

    while (1) {
        client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &client_len);
        pthread_t thread;
        pthread_create(&thread, NULL, handle_client, &client_sock);
        pthread_detach(thread);
    }

    close(server_sock);
    return 0;
}