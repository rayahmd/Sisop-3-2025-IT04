#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_WEAPONS 10

typedef struct {
    char name[50];
    int price;
    int damage;
    char passive[100];
    int passive_active;
} Weapon;

const char *orange_colour = "\033[38;5;214m";
const char *light_colour = "\033[38;5;222m";
const char *magenta_colour = "\033[38;5;212m";
const char *red_colour = "\033[38;5;196m";
const char *yellow_colour = "\033[38;5;226m";
const char *passive_colour = "\033[38;5;141m";
const char *pink_colour = "\033[38;5;218m";
const char *reset_colour = "\033[0m";

Weapon weapons[MAX_WEAPONS] = {
    {"Mossy Stone", 100, 20, "", 0},
    {"Flambeu", 150, 15, "Burn effect adds 5%-30% base attack as true damage", 1},
    {"Verdant Splitbow", 250, 12, "30% chance to bloom into 3 arrows, +10% per fail", 1},
    {"Detonark", 300, 16, "Every 3 rounds, explode 15% of max HP", 1},
    {"Codex Carmine", 400, 23, "30% chance to drain 10% HP for next strike", 1},
    {"Lunar Fang", 470, 27, "15%-30% chance for 100% crit hit", 1},
    {"Culling Spear", 500, 25, "Instant kill below 20% max HP", 1},
    {"Reaper's Covenant", 600, 41, "15% chance to reap 50% current HP", 1},
    {"Bloodshatter Blade", 750, 47, "Consumes 50% max HP if >120 on first attack", 1},
    {"Trispear of Ascension", 800, 55, "20% chance instant kill or 25% max HP", 1}
};

void get_weapon_list(char *buffer) {
    int offset = snprintf(buffer, 8192, "\n%s(𐑘   /)\n ˶ᵔ ᵕ ᵔ˶            W E A P O N   S H O P\n%s", orange_colour, reset_colour);
    for (int i = 0; i < MAX_WEAPONS && offset < 8192; i++) {
        int written = 0;
        if (weapons[i].passive_active) {
            written = snprintf(buffer + offset, 8192 - offset, "%s꒰%d ꒱%s %s%s%s %s- Price:%s %s%d gold%s%s, Damage:%s %s%d%s %s(Passive: %s)%s\n",
                    light_colour, i + 1, reset_colour, pink_colour, weapons[i].name, reset_colour, light_colour, reset_colour, yellow_colour, weapons[i].price, reset_colour, light_colour,
                    reset_colour, red_colour, weapons[i].damage, reset_colour, passive_colour, weapons[i].passive, reset_colour);
        } else {
            written = snprintf(buffer + offset, 8192 - offset, "%s꒰%d ꒱%s %s%s%s %s- Price:%s %s%d gold%s%s, Damage: %s%s%d%s\n",
                    light_colour, i + 1, reset_colour, pink_colour, weapons[i].name, reset_colour, light_colour, reset_colour, yellow_colour, weapons[i].price, reset_colour,
                    light_colour, reset_colour, red_colour, weapons[i].damage, reset_colour);
        }
        if (written < 0 || written >= 8192 - offset) break;
        offset += written;
    }
    snprintf(buffer + offset, 8192 - offset, "%s  ୨♡୧ ... Enter weapon number to buy (0 to cancel)%s", magenta_colour, reset_colour);
}

int buy_weapon(int weapon_id, int *gold, char *inventory, int *inventory_size) {
    if (weapon_id < 1 || weapon_id > MAX_WEAPONS) return 0;
    int idx = weapon_id - 1;
    if (*gold >= weapons[idx].price) {
        *gold -= weapons[idx].price;
        char temp[100];
        snprintf(temp, sizeof(temp), "%s|", weapons[idx].name);
        strcat(inventory, temp);
        (*inventory_size)++;
        return 1;
    }
    return 0;
}