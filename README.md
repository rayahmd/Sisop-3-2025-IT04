# PRAKTIKUM SISOP-3-2025-MH-IT04

KELOMPOK IT04
| No | Nama                              | NRP         |
|----|-----------------------------------|-------------|
| 1  | Raya Ahmad Syarif                 | 5027241041  |
| 2  | Salsa Bil Ulla         | 5027241052 |
| 3  | Adinda Cahya Pramesti   | 5027241117  |

# Pengantar

Laporan resmi ini dibuat untuk praktikum modul 3 yang dilaksanakan pada tanggal 28 April 2025 sampai dengan 3 Mei 2025. Praktikum Modul 3 ini terdiri dari empat soal dan dikerjakan oleh tiga orang anggota dengan pembagian tertentu.

Berikut ini pembagian pengerjaan dari kelompok IT04:

    Soal 1 dikerjakan oleh Raya Ahmad Syarif
    Soal 2 dikerjakan oleh Adinda Cahya Pramesti
    Soal 3 dikerjakan oleh Salsa Bil Ulla
    Soal 4 dikerjakan oleh Raya Ahmad Syarif

Sehingga dengan demikian, Pembagian bobot pengerjaan soal menjadi (Raya 50%, Salsa 25%, Dinda 25%).

Kelompok IT04 juga telah menyelesaikan tugas praktikum modul 3 yang telah diberikan. Dari hasil praktikum yang telah dilakukan sebelumnya, maka diperoleh hasil sebagaimana yang dituliskan pada setiap bab di bawah ini.
# Ketentuan
Berikut ini struktur dari repositori praktikum modul 3:

```
—soal_1:
    — image_client.c
    — image_server.c
—soal_2:
    — dispatcher.c
    — delivery_agent.c	
—soal_3:	
	  — dungeon.c
	  — shop.c/shop.h
	  — player.c
—soal_4:
    — system.c
    — hunter.c
—assets
```

## Soal 1
#### a. Download file secret.zip dan unzip ke client/secrets
```
wget --no-check-certificate 'https://docs.google.com/uc?export=download&id=15mnXpYUimVP1F5Df7qd_Ahbjor3o1cVw' -O secrets.zip
mkdir -p client/secrets
unzip secrets.zip -d client/secrets
```
Command di atas mendownload file secrets.zip menggunakan wget lalu membuat folder client/secrets dan mengunzip secrets.zip ke folder tersebut.

#### b. image_client harus bisa jalan di background dengan daemon
```
void daemonize(){
    pid_t pid = fork();
    
    if(pid < 0)
        exit(EXIT_FAILURE);
    if(pid > 0)
        exit(EXIT_SUCCESS);

    setsid();
    chdir("/");

    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
    openlog("image_server", LOG_PID, LOG_DAEMON);
}
```
Fungsi daemonize() mengubah program menjadi daemon, yaitu proses latar belakang yang berjalan tanpa terhubung ke terminal. Pertama, fork() digunakan untuk membuat proses anak; proses induk keluar agar hanya anak yang lanjut. Kemudian setsid() membuat sesi baru agar proses tidak lagi terikat terminal. chdir("/") memindahkan direktori kerja ke root untuk menghindari mengunci direktori aktif. Selanjutnya, input/output standar (stdin, stdout, stderr) ditutup agar daemon tidak terhubung ke terminal. Terakhir, openlog() digunakan untuk mencatat log dengan identifikasi "image_server" ke sistem log Linux.

#### c. image_client.c terhubung ke image_server untuk reverse text lalu decode from hex untuk disimpan dalam database server dengan nama file timestamp dan juga request download dari database.
```
void upload_text(int sock) {
    char filename[256], buffer[BUFFER_SIZE];
    printf("Masukkan nama file teks (contoh: input_1.txt): ");
    scanf("%s", filename);
    char filepath[512];
    snprintf(filepath, 512, "client/secrets/%s", filename);
    FILE *file = fopen(filepath, "r");
    if (!file) {
        printf("ERROR: File tidak ditemukan\n");
        return;
    }
    fseek(file, 0, SEEK_END);
    long fsize = ftell(file);
    fseek(file, 0, SEEK_SET);
    char *content = malloc(fsize + 1);
    fread(content, 1, fsize, file);
    fclose(file);
    content[fsize] = '\0';
    snprintf(buffer, BUFFER_SIZE, "UPLOAD_TEXT %s %s", filename, content);
    write(sock, buffer, strlen(buffer));
    int n = read(sock, buffer, BUFFER_SIZE - 1);
    if (n > 0) {
        buffer[n] = '\0';
        printf("%s\n", buffer);
    }
    free(content);
}
```
Fungsi `upload_text()` meminta pengguna memasukkan nama file teks, lalu mencoba membuka file tersebut dari folder `client/secrets/`. Jika file ditemukan, isinya dibaca sepenuhnya ke memori, kemudian dikemas menjadi pesan dengan format `"UPLOAD_TEXT <nama_file> <isi_file>"` dan dikirim ke server melalui socket. Setelah itu, client menunggu respon dari server dan menampilkannya ke layar. Akhirnya, memori yang digunakan untuk membaca isi file dibebaskan.
```
void download_jpeg(int sock) {
    char filename[256], buffer[BUFFER_SIZE];
    printf("Masukkan nama file JPEG (contoh: 1744403652.jpeg): ");
    scanf("%s", filename);
    snprintf(buffer, BUFFER_SIZE, "DOWNLOAD_JPEG %s", filename);
    write(sock, buffer, strlen(buffer));
    char filepath[512];
    snprintf(filepath, 512, "client/%s", filename);
    FILE *file = fopen(filepath, "wb");
    int n = read(sock, buffer, BUFFER_SIZE);
    if (n > 0) {
        if (strncmp(buffer, "ERROR", 5) == 0) {
            printf("%s\n", buffer);
        } else {
            fwrite(buffer, 1, n, file);
            printf("Sukses mengunduh %s\n", filename);
        }
    }
    fclose(file);
}
```
Fungsi `download_jpeg()` meminta pengguna memasukkan nama file JPEG yang ingin diunduh, lalu mengirim permintaan `"DOWNLOAD_JPEG <nama_file>"` ke server melalui socket. Setelah itu, client menyiapkan file output di folder `client/` dan menunggu data dari server. Jika data yang diterima diawali dengan `"ERROR"`, pesan kesalahan ditampilkan; jika tidak, data dianggap sebagai isi file JPEG dan ditulis ke file lokal. Setelah selesai, file ditutup.
#### e. Program dapat dijalankan dengan memasukkan perintah berkali-kali
````
int main() {
    int choice;
    while (1) {
        display_menu();
        scanf("%d", &choice);
        if (choice == 3) break;
        int sock = connect_to_server();
        if (sock < 0) continue;
        if (choice == 1) upload_text(sock);
        else if (choice == 2) download_jpeg(sock);
        close(sock);
    }
    return 0;
}
````
Menggunakan while true di function main sehingga program client bisa berjalan berkali-kali.

## Soal 2

## Soal 3 - **The Lost Dungeon**
Suatu pagi, anda menemukan jalan setapak yang ditumbuhi lumut dan hampir tertutup semak. Rasa penasaran membawamu mengikuti jalur itu, hingga akhirnya anda melihatnya: sebuah kastil tua, tertutup akar dan hampir runtuh, tersembunyi di tengah hutan. Gerbangnya terbuka seolah memanggilmu masuk.

Di dalam, anda menemukan pintu batu besar dengan simbol-simbol aneh yang terasa… hidup. Setelah mendorongnya dengan susah payah, anda pun menuruni tangga batu spiral yang dalam dan gelap. Di ujungnya, anda menemukan sebuah dunia baru: dungeon bawah tanah yang sudah tertinggal sangat lama.

Anda tidak tahu bagaimana anda dapat berada di situasi ini, tetapi didorong oleh rasa ingin tahu dan semangat, apa pun yang menunggumu di bawah sana, anda akan melawan. **(Author: Fico / purofuro)**

**a. Entering the dungeon**
**dungeon.c** akan bekerja sebagai **server** yang dimana **client** (player.c) dapat terhubung melalui RPC. dungeon.c akan **memproses** segala perintah yang dikirim oleh player.c. **Lebih dari 1 client** dapat mengakses **server**.

**b. Sightseeing** 
Anda melihat disekitar dungeon dan menemukan beberapa hal yang menarik seperti toko senjata dan pintu dengan aura yang cukup seram. Ketika player.c dijalankan, ia akan terhubung ke **dungeon.c** dan menampilkan sebuah **main menu** seperti yang dicontohkan di bawah ini (tidak harus mirip, dikreasikan sesuai kreatifitas masing-masing praktikan). 

**c. Status Check** 
Melihat bahwa terdapat sebuah toko senjata, anda mengecek status diri anda dengan harapan anda masih memiliki sisa uang untuk membeli senjata. Jika opsi **Show Player Stats** dipilih, maka program akan menunjukan Uang yang dimiliki (Jumlah dibebaskan), **senjata** yang sedang digunakan, **Base Damage**, dan **jumlah musuh** yang telah **dimusnahkan**.  

**d. Weapon Shop** 
Ternyata anda memiliki sisa uang dan langsung pergi ke toko senjata tersebut untuk membeli senjata. Terdapat **5 pilihan senjata** di toko tersebut dan beberapa dari mereka memiliki **_passive_** yang unik. Disaat opsi **_Shop_** dipilih, program akan menunjukan senjata apa saja yang dapat dibeli beserta **harga**, **damage**, dan juga **passive** (jika ada). **List senjata** yang ada dan dapat dibeli beserta **logic/command** untuk **membeli** senjata tersebut diletakan di code **shop.c/shop.h** yang nanti akan dipakai oleh **dungeon.c**. 
**Notes**: praktikan **_dibebaskan_** untuk **penamaan**, **harga**, **damage**, dan juga **passive** dari senjata-senjata yang ada. Yang penting harus terdapat **5 atau lebih** senjata dengan minimal **2 senjata** yang memiliki **passive**. 

**e. Handy Inventory** 
Setelah membeli senjata di toko tadi, anda membuka ransel anda untuk memakai senjata tersebut. Jika opsi **_View Inventory_** dipilih, program akan menunjukan senjata apa saja yang dimiliki dan dapat dipakai (jika senjata memiliki passive, tunjukan juga passive tersebut). 
Lalu apabila opsi **_Show Player Stats_** dipilih saat menggunakan weapon maka **Base Damage** player akan berubah dan jika memiliki **passive**, maka akan ada status tambahan yaitu **Passive**. 

**f. Enemy Encounter** 
Anda sekarang sudah siap untuk melewati pintu yang seram tadi, disaat anda memasuki pintu tersebut, anda langsung ditemui oleh sebuah musuh yang bukan sebuah manusia. Dengan tekad yang bulat, anda melawan musuh tersebut. Saat opsi **_Battle Mode_** dipilih, program akan menunjukan **health-bar** musuh serta angka yang menunjukan berapa darah musuh tersebut dan menunggu **input** dengan opsi **attack** untuk melakukan sebuah serangan dan juga **exit** untuk keluar dari **Battle Mode**. Apabila darah musuh berkurang, maka **health-bar** musuh akan berkurang juga. 
Jika darah musuh sudah 0, maka program akan menunjukan **rewards** berupa berapa banyak **gold** yang didapatkan lalu akan **muncul musuh** lagi. 

**g. Other Battle Logic**
**- Health & Rewards** 
Untuk darah musuh, seberapa banyak darah yang mereka punya dibuat secara **random**, contoh: **50-200 HP**. Lakukan hal yang sama untuk **rewards**.  
**- Damage Equation** 
Untuk **damage**, gunakan **base damage** sebagai kerangka awal dan tambahkan **rumus damage** apapun (dibebaskan, yang pasti perlu **random number** agar hasil damage bervariasi). Lalu buatlah logic agar setiap serangan memiliki kesempatan untuk **Critical** yang membuat **damage** anda 2x lebih besar. 
**- Passive** 
Jika senjata yang dipakai memiliki **Passive** setiap kali passive tersebut menyala, maka tunjukan bahwa **passive** tersebut aktif. 

**h. Error Handling**
Berikan **error handling** untuk opsi-opsi yang tidak ada.

### > Penyelesaian
#### dungeon.c
```
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
```

#### player.c
```
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define SERVER_IP "127.0.0.1"
#define PORT 8080

const char *magenta_foreground = "\033[38;5;206m";
const char *pink_foreground = "\033[38;5;212m";
const char *light_foreground = "\033[38;5;218m";
const char *red_foreground = "\033[38;5;196m";
const char *reset = "\033[0m";

void display_menu() {
    printf(
        "\n%sㅤ :¨·.·¨:            M     A     I     N               ￨𐑘__/,￨（｀＼%s\n"
        "%sㅤ  ˋ·.ꔫ ˊ            M     E     N     U             _.￨o o  ￨_ ）  ）%s\n"
        "%s-⋆-⋆-⋆-⋆-⋆-⋆-⋆-⋆-⋆-⋆-⋆-⋆-⋆-⋆-⋆୨♡୧⋆-⋆-⋆-⋆-⋆-⋆-⋆-⋆-⋆⋆-⋆-(((-⋆-(((⋆-⋆-⋆-⋆%s\n"
        "%s꒰ 1  ꒱ Show Player Stats\n꒰ 2  ꒱ Shop (Buy Weapons)\n꒰ 3  ꒱ View Inventory & Equip Weapons\n꒰ 4  ꒱ Battle Mode\n꒰ 5  ꒱ Exit Game%s\n"
        "%s/ ❦ . . Choose an option: %s",
        pink_foreground, reset, pink_foreground, reset, magenta_foreground, reset, light_foreground, reset, pink_foreground, reset
    );
}

const char *grads[] = {
    "\033[38;5;15m",
    "\033[38;5;225m",
    "\033[38;5;224m",
    "\033[38;5;222m",
    "\033[38;5;219m",
    "\033[38;5;218m",
    "\033[38;5;217m",
    "\033[38;5;216m",
    "\033[38;5;212m"
};


int main() {
    int sock;
    struct sockaddr_in server_addr;
    char buffer[8192];

    sock = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        printf("%s/ ❦ . . ꒰Connection failed. ꒱%s\n", red_foreground, reset);
        return 1;
    }

    printf(
        "\n%s       ᕱ    waddles*⠀⠀⣠⣠⣶⣿⣷⣿⣿⣿⣷⣷⣶⣤⣀⠀⠀⠀⠀⠀⠀⭑⠀⠀⠀⠀⠀%s"
        "\n%s ⭑              ⠀⠀⠀ ⣤⣾⣿⢿⣻⡽⣞⣳⡽⠚⠉⠉⠙⠛⢿⣶⣄⠀⠀ ⠀⠀⠀⠀⠀⠀⠀ hello! (˶ᵔ ᵕ ᵔ˶)%s"
        "\n%s⠀         ⭑       ⠀⣼⣿⣿⢻⣟⣧⢿⣻⢿⠀⠀⠀⠀⠀⠀⠀⠻⣿⣧⠀⠀ ⠀⠀⠀⭑⠀⠀ my name is piplup!%s"
        "\n%s⠀  𐪜             ⢀⣾⣿⡿⠞⠛⠚⠫⣟⡿⣿⠀⠀⠀⠀⠀⠀⠀⠀⠘⢿⣧⠀⠀⠀⠀⠀⠀⠀%s"
        "\n%s⠀               ⠀⣼⣿⡟⠀⠀⠀⠀⠀⠈⢻⡽⣆⠀⠀⣴⣷⡄⠀⠀⠀⠘⣿⡆⠀⠀⣀⣠⣤⡄                ⭑%s"
        "\n%s⠀       ⭑        ⣿⣿⠁⠀⠀⠀⠀⠀⠀⠈⣿⠿⢷⡀⠘⠛⠃⠀⠀⠀⠀⣿⣅⣴⡶⠟⠋⢹⣿       ʬʬ%s"
        "\n%s⠀ ⭑             ⠀⢻⣿⡀⠀⠀⠀⣾⣿⡆⠀⢿⣴⣴⡇⠀⠀⠀⠀⠀⠀⢠⡟⠋⠁⠀⠀⠀⢸⣿%s"
        "\n%s⠀⠀               ⠈⢿⣇⠀⠀⠀⠀⠉⠁⠀⠀⠉⠉⠀⠀⠀⠀⠀⠀⢀⡾⠁⠀⠀⠀⠀⠀⣾⡏   ⭑%s"
        "\n%s⠀⠀⠀      ٪        ⠈⢿⣦⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢠⢸⠁⠀⠀⠀⠀⠀⣼⡟⠀           ⭑%s"
        "\n%s⠀⠀⠀⠀               ⠀⣹⣿⣶⣤⣀⡀⠀⠀⠀⠀⠀⣀⠀⠀⠂⠁⠀⠐⢧⡀⠀⢀⣾⠟⠀⠀               ⁇%s"
        "\n%s⠀⠀ ⭑         ⭑   ⢀⣰⣾⠟⠉⠀⠀⠉⠉⠀⠐⠂⠀⠁⠁⠀⠀⠀⠀⠀⠀⠈⢿⣶⡟⠋⠀⠀⠀       ⭑%s"
        "\n%s               ⣠⣶⡿⠋⠁⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠈⣿⡆⠀⠀⠀⠀ ⭑%s"
        "\n%s         ⭑     ⢻⣧⣄⠀⠀⠀⢰⡇⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢸⣿⠀⠀⠀ ⠀           ⭑%s"
        "\n%s    ⁉          ⠀⠉⠛⠿⣷⣶⣾⡇⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢸⣿⠀⠀⠀⠀%s"
        "\n%s⠀⠀⠀⠀⠀               ⠀⣿⡇⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣤⣤⣾⣿⠀⠀⠀⠀   𑊂    ⭑%s"
        "\n%s⠀⭑⠀⠀⠀⠀    ⭑         ⠀⢹⣿⣿⣿⣿⣷⣦⡀⠀⢀⣀⠀⠀⠀⣠⣴⣿⣿⣿⣿⣷⠀⠀  ⠀⠀            ⭑%s"
        "\n%s⠀            ⁈  ⠀⠀⠀⠀⠀⠀⠻⢿⣿⣿⣿⣿⠿⠿⠿⠿⠿⠿⠿⠿⣿⣿⣿⠿⠟⠁⠀⠀⠀⠀ ⭑                 ǃ%s"
        "\n%s     ⭑            ⭑                                  ⭑%s",
        grads[0], reset, grads[0], reset, grads[1], reset, grads[1], reset, grads[2], reset, grads[2], reset, grads[3], reset, grads[3], reset, grads[4], reset,
        grads[4], reset, grads[5], reset, grads[5], reset, grads[6], reset, grads[6], reset, grads[7], reset, grads[7], reset, grads[8], reset, grads[8], reset
    );

    while (1) {
        display_menu();
        char input[100];
        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = '\0';

        write(sock, input, strlen(input));
        int n = read(sock, buffer, sizeof(buffer) - 1);
        if (n <= 0) break;
        buffer[n] = '\0';
        printf("%s\n", buffer);

        if (strcmp(input, "5") == 0) break;

        if (strcmp(input, "2") == 0 || strcmp(input, "3") == 0) {
            printf("%s/ ❦ . . Enter choice: %s", pink_foreground, reset);
            fgets(input, sizeof(input), stdin);
            input[strcspn(input, "\n")] = '\0';
            write(sock, input, strlen(input));
            n = read(sock, buffer, sizeof(buffer) - 1);
            if (n <= 0) break;
            buffer[n] = '\0';
            printf("%s\n", buffer);
        }

        if (strcmp(input, "4") == 0) {
            char exit_phrase[100];
            sprintf(exit_phrase, "You left the battle.");
            while (strstr(buffer, exit_phrase) == NULL) {
                printf("%s> %s", light_foreground, reset);
                fgets(input, sizeof(input), stdin);
                input[strcspn(input, "\n")] = '\0';
                write(sock, input, strlen(input));
                n = read(sock, buffer, sizeof(buffer) - 1);
                if (n <= 0) break;
                buffer[n] = '\0';
                printf("%s\n", buffer);
            }
        }
    }

    close(sock);
    return 0;
}
```

#### shop.c
```
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
```

### > Penjelasan
#### a. Entering the dungeon
**`dungeon.c` - main()**
```
int server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr));
    listen(server_sock, MAX_CLIENTS);
```
Membuat socket TCP (`SOCK_STREAM`) untuk komunikasi, lalu melakukan binding ke port 8080 (`PORT` didefinisikan dengan `#define PORT 8080`). Serta menggunakan `listen()` untuk server menerima hingga `MAX_CLIENTS` koneksi.
```
while (1) {
        client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &client_len);
        pthread_t thread;
        pthread_create(&thread, NULL, handle_client, &client_sock);
        pthread_detach(thread);
    }
```
`accept()` akan menunggu koneksi dari client dengan setiap koneksi baru ditangani dalam thread sendiri melalui `pthread_create()`. Lalu agar thread dilepas otomatis saat selesai (tidak perlu `join()`) dapat menggunakan `pthread_detach()`.

**`player.c` - main()**
```
int sock;
    struct sockaddr_in server_addr;
    char buffer[8192];

    sock = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        printf("%s/ ❦ . . ꒰Connection failed. ꒱%s\n", red_foreground, reset);
        return 1;
    }
```
Pada kode di atas, `socket()` dan `connect()` digunakan untuk menghubungkan client ke server. Server IP di-hardcode sebagai `127.0.0.1` dan port sebagai `8080`.

#### b. Sightseeing
**`player.c` - display_menu()**
```
void display_menu() {
    printf(
        "\n%sㅤ :¨·.·¨:            M     A     I     N               ￨𐑘__/,￨（｀＼%s\n"
        "%sㅤ  ˋ·.ꔫ ˊ            M     E     N     U             _.￨o o  ￨_ ）  ）%s\n"
        "%s-⋆-⋆-⋆-⋆-⋆-⋆-⋆-⋆-⋆-⋆-⋆-⋆-⋆-⋆-⋆୨♡୧⋆-⋆-⋆-⋆-⋆-⋆-⋆-⋆-⋆⋆-⋆-(((-⋆-(((⋆-⋆-⋆-⋆%s\n"
        "%s꒰ 1  ꒱ Show Player Stats\n꒰ 2  ꒱ Shop (Buy Weapons)\n꒰ 3  ꒱ View Inventory & Equip Weapons\n꒰ 4  ꒱ Battle Mode\n꒰ 5  ꒱ Exit Game%s\n"
        "%s/ ❦ . . Choose an option: %s",
        pink_foreground, reset, pink_foreground, reset, magenta_foreground, reset, light_foreground, reset, pink_foreground, reset
    );
}
```
**`player.c` - main()**
```
while (1) {
        display_menu();
        char input[100];
        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = '\0';

        write(sock, input, strlen(input));
        int n = read(sock, buffer, sizeof(buffer) - 1);
        if (n <= 0) break;
        buffer[n] = '\0';
        printf("%s\n", buffer);

        if (strcmp(input, "5") == 0) break;

        if (strcmp(input, "2") == 0 || strcmp(input, "3") == 0) {
            printf("%s/ ❦ . . Enter choice: %s", pink_foreground, reset);
            fgets(input, sizeof(input), stdin);
            input[strcspn(input, "\n")] = '\0';
            write(sock, input, strlen(input));
            n = read(sock, buffer, sizeof(buffer) - 1);
            if (n <= 0) break;
            buffer[n] = '\0';
            printf("%s\n", buffer);
        }

        if (strcmp(input, "4") == 0) {
            char exit_phrase[100];
            sprintf(exit_phrase, "You left the battle.");
            while (strstr(buffer, exit_phrase) == NULL) {
                printf("%s> %s", light_foreground, reset);
                fgets(input, sizeof(input), stdin);
                input[strcspn(input, "\n")] = '\0';
                write(sock, input, strlen(input));
                n = read(sock, buffer, sizeof(buffer) - 1);
                if (n <= 0) break;
                buffer[n] = '\0';
                printf("%s\n", buffer);
            }
        }
```
Fungsi untuk default main menu diletakkan di `player.c` dengan desain imut dan berwarna nuansa pink. Setiap opsi dari main menu terhubung dengan fitur masing-masing dan menu akan terus tampil karena berada dalam loop `while (1)` di `main()`

**`player.c` - main()**
```
fgets(input, sizeof(input), stdin);
input[strcspn(input, "\n")] = '\0';

write(sock, input, strlen(input));
```
Ini mengirim input ke server `dungeon.c`, yang akan memproses sesuai dengan perintah.

**`dungeon.c` - *handle_client()**
```
int option = atoi(buffer);
switch (option) {
    case 1: get_player_stats(...); break;
    case 2: get_weapon_list(...); break;
    case 3: get_inventory(...); break;
    case 4: start_battle(...); break;
    case 5: strcpy(response, "Goodbye!"); ... break;
    default: strcpy(response, "Invalid option. Please try again.");
}
```
Setelah menerima input angka dari client, server akan memanggil fungsi yang sesuai. Validasi juga dilakukan: jika input bukan 1–5, muncul pesan error.

#### c. Status Check
**`dungeon.c` - get_player_stats()**
```
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
```
Mengakses data pemain dari array `players[]` berdasarkan `client_id`. `snprintf` digunakan untuk menyusun tampilan status dengan gaya visual. Jika pemain menggunakan senjata dengan passive, maka `Passive:` juga ditampilkan.

**`dungeon.c` - *handle_client()**
```
case 1:
    get_player_stats(client_id, response);
    break;
```
Fungsi tersebut dipanggil dari `handle_client()` saat pemain memilih opsi 1 dari menu.

**`dungeon.c` - struct**
```
typedef struct {
    int gold;
    char equipped_weapon[50];
    int base_damage;
    int kills;
    char passive[100];
    ...
} Player;
```
Semua data status pemain tersimpan dalam struct `Player`, lalu diakses oleh `get_player_stats()`.

#### d. Weapon Shop
**`shop.c` - weapons[]**
```
Weapon weapons[MAX_WEAPONS] = {
    {"Mossy Stone", 100, 20, "", 0},
    {"Flambeu", 150, 15, "Burn effect adds 5%-30% base attack as true damage", 1},
    {"Verdant Splitbow", 250, 12, "30% chance to bloom into 3 arrows, +10% per fail", 1},
    {"Detonark", 300, 16, "Every 3 rounds, explode 15% of max HP", 1},
    {"Codex Carmine", 400, 23, "30% chance to drain 10% HP for next strike", 1},
    ...
};
```
Senjata disimpan dalam array `Weapon[]` dengan properti `name`, `price`, `damage`, dan `passive`. `passive_active` = 1 menandakan senjata punya efek spesial.

**`shop.c` - get_weapon_list()**
```
void get_weapon_list(char *buffer) {
    for (int i = 0; i < MAX_WEAPONS; i++) {
        if (weapons[i].passive_active) {
            snprintf(..., "(Passive: %s)", weapons[i].passive);
        } else {
            snprintf(...); // tanpa passive
        }
    }
}
```
Menggabungkan seluruh daftar senjata ke dalam buffer untuk dikirim ke client. Deskripsi mencakup nama, harga, damage, dan passive jika ada.

**`shop.c` - buy_weapon()**
```
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
```
Mengecek apakah gold cukup terlebih dahulu. Jika ya, senjata dimasukkan ke string inventory (dipisahkan dengan `|`) dan gold dikurangi.

**`dungeon.c` - *handle_client()**
```
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
```
Server akan mengirim daftar senjata ke client. Lalu client akan menerima pilihan senjata (angka). Sistem memanggil buy_weapon() untuk proses pembelian.

**`player.c` - main()**
```
if (strcmp(input, "2") == 0 || strcmp(input, "3") == 0) {
    printf("%s/ ❦ . . Enter choice: %s", pink_foreground, reset);
    fgets(input, sizeof(input), stdin);
    input[strcspn(input, "\n")] = '\0';
    write(sock, input, strlen(input));
    n = read(sock, buffer, sizeof(buffer) - 1);
    if (n <= 0) break;
    buffer[n] = '\0';
    printf("%s\n", buffer);
}
```
Program mengirimkan angka 2 ke server untuk meminta daftar senjata dari shop.c, lalu menampilkannya di terminal. Setelah pemain memilih senjata, input dikirim kembali ke server, dan hasil transaksi (berhasil atau gagal) diterima dan ditampilkan. Dengan ini, player.c berperan sebagai antarmuka interaktif untuk navigasi dan pembelian senjata di dungeon.

#### e. Handy Inventory
**`dungeon.c` - get_inventory()**
```
void get_inventory(int client_id, char *buffer) {
    Player *p = &players[client_id];
    strncpy(inventory_copy, p->inventory, sizeof(inventory_copy) - 1);
    char *token = strtok(inventory_copy, "|");
    while (token != NULL) {
        ...
        if (weapon_idx >= 0 && weapons[weapon_idx].passive_active) {
            snprintf(temp, ..., "(Passive: %s)%s%s\\n", weapons[weapon_idx].passive, ...);
        } else {
            snprintf(temp, ..., " %s\\n", token);
        }
        ...
    }
}
```
Inventory pemain disimpan sebagai string `"weapon1|weapon2|..."`. Setiap senjata dicek apakah memiliki passive. Senjata yang sedang dipakai akan ditandai `(EQUIPPED)`.

**`dungeon.c` - equip_weapon()**
```
int equip_weapon(int client_id, int weapon_id) {
    ...
    if (idx == weapon_id) {
        strcpy(p->equipped_weapon, start);
        p->base_damage = weapons[i].damage;
        strcpy(p->passive, weapons[i].passive);
        return 1;
    }
    ...
}
```
Setelah pemain memilih senjata dari inventory, senjata tersebut dijadikan equipped. `base_damage` dan `passive` akan langsung berubah.

**`player.c` - main()**
```
if (strcmp(input, "3") == 0) {
    write(sock, input, strlen(input));     
    read(sock, buffer, ...);             
    printf("%s\n", buffer);

    printf("Enter choice: ");
    fgets(input, ...);                     
    write(sock, input, strlen(input));      
    read(sock, buffer, ...);                
    printf("%s\n", buffer);
}
```
`player.c` menerima daftar inventory dari server. User pilih senjata untuk dipakai. Server update status dan mengirim feedback.

#### f. Enemy Encounter
**`dungeon.c` - start_battle()**
```
void start_battle(int client_id) {
    Player *p = &players[client_id];
    p->in_battle = 1;
    p->enemy_max_hp = 50 + (rand() % 151);
    p->enemy_hp = p->enemy_max_hp;
    ...
}
```
Mengatur status pemain sebagai in battle dan musuh baru dibuat dengan HP acak antara 50–200.

**`dungeon.c` - display_enemy_status()**
```
void display_enemy_status(int client_id, char *buffer, int is_new_battle) {
    int filled_bars = (p->enemy_hp * 50) / p->enemy_max_hp;
    ...
    snprintf(buffer, ..., "█ ... %d/%d HP", p->enemy_hp, p->enemy_max_hp);
}
```
Menampilkan visual bar HP musuh, panjang 50 bar, warna berbeda berdasarkan persentase HP. Output digunakan setiap kali battle dimulai/diperbarui.

**`dungeon.c` - attack_enemy()**
```
void attack_enemy(int client_id, char *buffer) {
    int damage = base_damage + random;
    ...
    if (p->enemy_hp <= 0) {
        p->kills++;
        p->gold += 50 + (rand() % 101);
        start_battle(client_id);  
        ...
    } else {
        display_enemy_status(...);  
    }
}
```
Hitung damage, kurangi HP musuh. Jika HP musuh 0, tampilkan reward dan langsung spawn musuh baru. Jika belum mati, tampilkan HP terbaru.

**`player.c` - main()**
```
if (strcmp(input, "4") == 0) {
    write(sock, input, strlen(input)); 
    read(sock, buffer, ...);  

    while (musuh belum mati/kabur) {
        fgets(input, ...);      
        write(sock, input, ...);
        read(sock, buffer, ...);
        printf("%s\n", buffer);
    }
}
```
Pemain bisa mengetik `attack` atau `exit` berulang kali. Respons dari server ditampilkan setiap serangan.

#### g. Other Battle Logic
**`dungeon.c`**
```
p->enemy_max_hp = 50 + (rand() % 151);
p->enemy_hp = p->enemy_max_hp;

int reward = 50 + (rand() % 101);
p->gold += reward;
```
HP musuh dan reward ditentukan dengan `rand()` agar tiap pertarungan terasa unik.

```
int damage = p->base_damage + (rand() % 10); // base + 0–9
int is_crit = rand() % 100 < 20; // 20% chance
if (is_crit) damage *= 2;
```
Damage dasar ditambah random 0–9. 20% kemungkinan menjadi critical hit (2x damage). Critical ditampilkan dengan efek warna dan pesan khusus.

```
if (strcmp(p->equipped_weapon, "Flambeu") == 0) {
    bonus_dmg = (p->base_damage * p->flambeu_burn_stack) / 100;
}

else if (strcmp(p->equipped_weapon, "Verdant Splitbow") == 0) {
    int bloom_chance = 30 + (fail_count * 10);
    if (rand() % 100 < bloom_chance) {
        damage *= 3;
        fail_count = 0;
    } else {
        fail_count++;
    }
}
```
Masing-masing senjata punya efek unik saat digunakan menyerang.
Contoh efek passive:
- Flambeu: burn stack (bonus true damage)
- Verdant Splitbow: chance 30–60% untuk triple damage
- Codex Carmine: drain HP musuh dan simpan untuk serangan berikutnya
- Trispear: 20% chance instant kill, jika gagal tetap beri bonus damage
- Culling Spear: auto-kill jika HP musuh di bawah 20%
Semua passive dikontrol langsung di dalam fungsi `attack_enemy()`.

#### h. Error Handling
**`dungeon.c`**
```
switch (option) {
    case 1: ...; break;
    case 2: ...; break;
    ...
    default:
        strcpy(response, "Invalid option. Please try again.\n");
}
```
Jika input tidak sesuai 1–5, server kirim pesan error ke client.

```
if (players[client_id].in_battle) {
    if (strcmp(buffer, "attack") == 0) {
        attack_enemy(...);
    } else if (strcmp(buffer, "exit") == 0) {
        ...
    } else {
        strcpy(response, "Invalid command. Type 'attack' or 'exit'.\n");
    }
}
```
Saat battle, hanya command `attack` dan `exit` yang diterima. Selain itu akan ditolak dengan pesan error.

```
if (weapon_id == 0) {
    strcpy(response, "Purchase cancelled.\n");
} else if (buy_weapon(...)) {
    strcpy(response, "Weapon purchased successfully!\n");
} else {
    strcpy(response, "Not enough gold or invalid weapon.\n");
}
```
```
if (equip_weapon(...) == 0) {
    strcpy(response, "Invalid weapon ID.\n");
}
```
Saat beli senjata, dicek apakah uang cukup dan ID valid. Saat equip senjata, dicek apakah ID sesuai dengan inventory.

### > Dokumentasi
![image](https://github.com/user-attachments/assets/9289468c-6b98-46ba-ab79-60549f98056a)
![image](https://github.com/user-attachments/assets/b21e25a4-983a-4e36-a1e5-f2cf27b3785a)
![image](https://github.com/user-attachments/assets/9a51b4ed-a6a9-451b-a4f7-e4df47e081f4)
![image](https://github.com/user-attachments/assets/a4569761-b0cc-467b-b788-58fc6e7b78e3)
![image](https://github.com/user-attachments/assets/abeda644-f117-4d38-adbf-bfbf2a81ce9e)
![image](https://github.com/user-attachments/assets/9414b5a7-5a99-4695-a188-60dc49db33de)
![image](https://github.com/user-attachments/assets/859fd885-1faf-4771-995a-e785fb5d7624)
![image](https://github.com/user-attachments/assets/a2558465-66de-4499-8c1e-22233e1c0137)
![image](https://github.com/user-attachments/assets/bf2bd897-b943-49cc-add5-6508d297b174)
![image](https://github.com/user-attachments/assets/16d77209-86d0-42f2-ab63-8bf4617c0732)
![image](https://github.com/user-attachments/assets/292366f2-a28c-4b85-868b-265dfe65e965)

## Soal 4


### > Penjelasan
#### a. Buat file system.c sebagai server dan hunter.c sebagai client
```
key_t key = ftok("/tmp", 'S');
int shmid = shmget(key, sizeof(SystemData),
                  IPC_CREAT | IPC_EXCL | 0666);
SystemData *sys = shmat(shmid, NULL, 0);
sys->num_hunters = 0;
sys->num_dungeons = 0;
sys->current_notification_index = 0;

```
Cuplikan code di atas merupakan code dari system.c yang membuat main shared memory. `ftok("/tmp", 'S');` menghasilkan unique key untuk shared memory utama. shmget() digunakan untuk membuat shared memory utama yang digunakan oleh sistem untuk menyimpan status seperti jumlah hunter, dungeon, dan lainnya. 

```
key_t key = ftok("/tmp", 'S');
int shmid = shmget(key, sizeof(SystemData), 0666);
SystemData *sys = shmat(shmid, NULL, 0);
```
Hunter.c mengakses shared memory utama yang telah dibuat oleh system menggunakan key 'S'.  Penggunaan shmget() untuk menghubungkan ke shared memory tersebut.
```
hunter->shm_key = ftok("/tmp/hunter_system", 1000 + sys->num_hunters);
int h_shmid = shmget(hunter->shm_key, sizeof(Hunter),
                    IPC_CREAT | 0666);
Hunter *hmem = shmat(h_shmid, NULL, 0);
*hmem = *hunter;
```
Code di atas di line `ftok("/tmp/hunter_system", 1000 + sys->num_hunters)` menghasilkan shared memory untuk setiap hunter yang didaftarkan. 

##### b. Buat fitur register yang membuat unique key setiap hunter dan stat awal (Level=1, EXP=0, ATK=10, HP=100, DEF=5) dan login pada file hunter.c. Data hunter yang disimpan mempunyai shared memory sendiri yang terhubung dengan sistem.

```

void register_hunter(char *username) {
    FILE *fp = fopen("/tmp/hunter_system", "a");
    if (fp == NULL) {
        perror("fopen /tmp/hunter_system");
        return;
    }
    fclose(fp);

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
    hunter->shm_key = ftok("/tmp/hunter_system", 1000 + system_data->num_hunters);
    if (hunter->shm_key == -1) {
        perror("ftok");
        shmdt(system_data);
        return;
    }

    int hunter_shmid = shmget(hunter->shm_key, sizeof(struct Hunter), IPC_CREAT | 0666);
    if (hunter_shmid == -1) {
        perror("shmget hunter");
        return;
    }

    struct Hunter *hunter_shm = (struct Hunter *)shmat(hunter_shmid, NULL, 0);
    if (hunter_shm == (void *)-1) {
        perror("shmat hunter");
        shmctl(hunter_shmid, IPC_RMID, NULL);
        shmdt(system_data);
        return;
    }
    *hunter_shm = *hunter;
    shmdt(hunter_shm);
    system_data->num_hunters++;
    printf("Hunter %s registered.\n", username);
    shmdt(system_data);
}
```
Pada function di atas pada line `fopen("/tmp/hunter_system", "a");` digunakan untuk mengecek apakah ada file /tmp/hunter_system agar ftok bisa membuat unique key untuk setiap hunter. Selanjutnya, mengakses system key yang sama dengan system.c untuk mengakses memory segment yang berisi array hunters.

```
struct Hunter *h = &system_data->hunters[num_hunters];  
strcpy(h->username, username);  
h->level = 1; h->exp = 0; h->atk = 10; h->hp = 100; h->def = 5;  
h->banned = 0;

```
Pada cuplikan di sini untuk mengisi data hunter baru ke shared memory utama dan mengisi stats sesuai permintaan soal.

```
h->shm_key = ftok("/tmp/hunter_system", 1000 + num_hunters);  
hunter_shmid = shmget(h->shm_key, sizeof(Hunter), IPC_CREAT | 0666);
```
Membuat shared memory per hunter sehingga bisa di-attach dan detach secara terpisah. 
#### c. Display stats hunter untuk system.c yang menampilkan nama, level, exp, atk, hp, def, dan status banned atau tidak.
```
void display_hunters() {
    printf("Registered Hunters:\n");
    for (int i = 0; i < system_data->num_hunters; i++) {
        struct Hunter *h = &system_data->hunters[i];
        printf("Username: %s, Level: %d, EXP: %d, ATK: %d, HP: %d, DEF: %d, Banned: %s\n",
               h->username, h->level, h->exp, h->atk, h->hp, h->def, h->banned ? "Yes" : "No");
    }
}
```
Function di atas melakukan looping sesuai dengan hunters yang telah terdaftar pada shared memory. Dari situ akan menampilkan semua hunter yang ada beserta stats yang mereka miliki.
#### d. Buatlah dungeon dengan random stats seperti level, atk, hp, def, exp sesuai dengan kriteria yang sudah ditetapkan. Dungeon yang di-generate akan disimpan dalam shared memory sendiri yang berbeda dan dapat diakses oleh hunters.
```
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
```

Pada awalnya function di atas akan mengecek apakah dungeon sudah melebihi max dungeons atau belum. Lalu jika masih belum melebihi, akan mendapatkan pointer ke `system_data->dungeons[num_dungeons]` yaitu array untuk diisi dungeon. Selanjutnya akan diinisiasi atribut seperti nama, level, stas lainnya yang akan diisi secara random. Selanjutnya key akan digenerate di shared memory. shmget() akan dipanggil untuk membuat segment baru dan shmat untuk attach ke segment tersebut kemudian shmdt() dipanggil untuk melepaskan attachment tadi. Selanjutnya, akan mengkonfirmasi dungeon created untuk menandai dungeon sukses dibuat.


#### e. Buatlah fitur menampilkan dungeons yang telah terbuat pada system.c

```
void display_dungeons() {
    printf("Available Dungeons:\n");
    for (int i = 0; i < system_data->num_dungeons; i++) {
        struct Dungeon *d = &system_data->dungeons[i];
        printf("Name: %s, Min Level: %d, EXP: %d, ATK: %d, HP: %d, DEF: %d, Key: %d\n",
               d->name, d->min_level, d->exp, d->atk, d->hp, d->def, d->shm_key);
    }
}
```
Function di atas digunakan untuk menampilkan dungeon yang telah tergenerate. Pada awalnya akan memulai loop pada shared memory untuk looping seluruh dungeon, lalu akan menampilkan seluruh dungeon beserta statsnya.

#### f. Menampilkan dungeon berdasarkan level hunter

```
int display_ur_dungeons(struct Hunter *cur_hunter, int *dungeon_i, int mode) {
    printf("=== Available Dungeons for Hunter %s level: %d ===\n", cur_hunter->username, cur_hunter->level);
    printf("------------------------------------------------\n");
    printf("| No | Name          | Min Lv | EXP  | ATK | HP  | DEF |\n");
    printf("------------------------------------------------\n");
    int count = 0;
    for (int i = 0; i < system_data->num_dungeons; i++) {
        struct Dungeon *d = &system_data->dungeons[i];
        if (d->min_level <= cur_hunter->level) {
            if (d->name[0] == '\0') {
                printf("| %2d | %-13s | %6d | %4d | %3d | %3d | %3d |\n",
                       count + 1, "(Unnamed)", d->min_level, d->exp, d->atk, d->hp, d->def);
            } else {
                printf("| %2d | %-13s | %6d | %4d | %3d | %3d | %3d |\n",
                       count + 1, d->name, d->min_level, d->exp, d->atk, d->hp, d->def);
            }
            if (mode == 1 && dungeon_i != NULL) {
                dungeon_i[count] = i;
            }
            count++;
        }
    }
    if (count == 0) {
        printf("|                No dungeons available right now                  |\n");
    }
    printf("------------------------------------------------\n");
    return count;
}
```
Sama seperti display_dungeons hanya saja terdapat perkondisian di line ` if (d->min_level <= cur_hunter->level)` yang menampilkan dungeon berdasarkan level hunter atau di bawahnya.

#### g. Buatlah sistem raid dungeon yang ketika hunter berhasil mengalahkan dungeon akan mendapat reward exp dan bisa naik level apabila exp hunter mencapai 500 exp.

```
void raid(){
    if(cur_hunter->banned){
        printf("You're still banned from raiding... \n");
        return;
    }
    int dungeon_i[MAX_DUNGEONS];
    int dungeon_count = display_ur_dungeons(cur_hunter, dungeon_i, 1);
    if(dungeon_count == 0){ printf("No dungeons available right now!"); return;}
    
    int choice;
    printf("Enter dungeon number (1-%d): ", dungeon_count);
    scanf("%d", &choice);
    getchar(); 
    if (choice < 0 || choice > dungeon_count) {
        printf("Invalid.\n");
        return;
    }

    int choosed_dungeon = dungeon_i[choice - 1];
    struct Dungeon *dungeon = &system_data->dungeons[choosed_dungeon];

    int hunter_stats = cur_hunter->atk + cur_hunter->hp + cur_hunter->def;
    int dungeon_stats = dungeon->atk + dungeon->hp + dungeon->def;
    printf("Battle: %s (Stats: %d) vs %s (Stats: %d)\n",
           cur_hunter->username, hunter_stats, dungeon->name, dungeon_stats);

    if (hunter_stats > dungeon_stats) {
        // kalo si hunter win
        cur_hunter->exp += dungeon->exp;
        cur_hunter->atk += dungeon->atk / 2; 
        cur_hunter->hp += dungeon->hp / 2;   
        cur_hunter->def += dungeon->def / 2; 
        int dungeon_shmid = shmget(dungeon->shm_key, sizeof(struct Dungeon), 0666);
        shmctl(dungeon_shmid, IPC_RMID, NULL);

        for (int i = choosed_dungeon; i < system_data->num_dungeons - 1; i++) {
            system_data->dungeons[i] = system_data->dungeons[i + 1];
        }
        system_data->num_dungeons--;

        int hunter_shmid = shmget(cur_hunter->shm_key, sizeof(struct Hunter), 0666);
        struct Hunter *hunter_shm = (struct Hunter *)shmat(hunter_shmid, NULL, 0);
        *hunter_shm = *cur_hunter;
        shmdt(hunter_shm);

        printf("Raid success! You defeated %s and gained rewards: EXP+%d, ATK+%d, HP+%d, DEF+%d\n",
               dungeon->name, dungeon->exp, dungeon->atk / 2, dungeon->hp / 2, dungeon->def / 2);
    } else {
        // kalo hunter kalah
        int damage = dungeon->atk;
        cur_hunter->hp -= damage;
        printf("Raid failed! You took %d damage from %s. HP remaining: %d\n",
               damage, dungeon->name, cur_hunter->hp);


        int hunter_shmid = shmget(cur_hunter->shm_key, sizeof(struct Hunter), 0666);
        struct Hunter *hunter_shm = (struct Hunter *)shmat(hunter_shmid, NULL, 0);
        *hunter_shm = *cur_hunter;
        shmdt(hunter_shm);

        if (cur_hunter->hp <= 0) {
            printf("You have been defeated and removed from the system.\n");
            shmctl(hunter_shmid, IPC_RMID, NULL);
            for (int i = 0; i < system_data->num_hunters; i++) {
                if (strcmp(system_data->hunters[i].username, cur_hunter->username) == 0) {
                    for (int j = i; j < system_data->num_hunters - 1; j++) {
                        system_data->hunters[j] = system_data->hunters[j + 1];
                    }
                    system_data->num_hunters--;
                    break;
                }
            }
            shmdt(cur_hunter);
            shmdt(system_data);
            exit(0);
        }
    }
}

```

Pada awalnya akan mengecek status banned hunter yang mana hunter tidak akan bisa raid apabila status bannednya yes. Jika bisa raid akan menampilkan dungeon dan mempersilahkan hunter untuk memilih dungeon yang ingin diraid. Terdapat kondisi apabila hunter menang akan mendapat reward exp, jika kalah akan terhapus dari system dengan shmctl().

#### h. Buatlah fitur battle untuk pertarungan antar hunter yang mengadu total stats. Jika hunter menang stats hunter yang kalah akan ditambahkan ke statsnya.

```

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
            int ur_total = cur_hunter->atk + cur_hunter->hp + cur_hunter->def;
            int opp_total = opponent->atk + opponent->hp + opponent->def;
            printf("Your stats: ATK=%d, HP=%d, DEF=%d, Total=%d\n", cur_hunter->atk, cur_hunter->hp, cur_hunter->def, ur_total);
            printf("Opponent stats: ATK=%d, HP=%d, DEF=%d, Total=%d\n", opponent->atk, opponent->hp, opponent->def, opp_total);
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
```
Function di atas merupakan fitur battle yang membandingkan total stats antar hunter. Terdapat perkondisian berdasarkan kondisi total stats hunter. Apabila hunter yang melawan menang, akan mendapat reward yaitu exp dan stats lawan. Apabila hunter kalah, lawan akan mengambil statsnya dan hunter yang melawan akan terhapus dari system dengan shmdt().

#### i. Buat fitur banned serta unbannednya. Jika dibanned tidak akan bisa raid dan battle.
```
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

```
Pada dua function di atas merupakan fitur banned dan unbanned, cara kerjanya adalah akan melakukan looping untuk mencari username hunter yang akan dibanned atau unbanned. Apabila username sudah ditemukan akan mengubah kondisi banned yang awalnya 0 menjadi 1, akan terjadi sebaliknya pada unbanned.

#### j. Buatlah fitur untuk reset stats hunter ke default.
```
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
```
Pada function reset akan looping untuk menemukan hunter yang akan direset berdasarkan usernamenya. Apabila sudah ditemukan, stats akan diset lagi sesuai dengan default stats yang sudah ditentukan.

#### k. Fitur notifikasi dungeon sesuai dengan level hunter yang menampilkan dungeon setiap 3 detik.


```
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
```
Function di atas digunakan untuk menghidupkan dan mematikan notifikasi berdasarkan permintaan hunter.

```
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
```
function ini menerima parameter dari toggle notif, apabila notification_running = 1 akan membersihkan layar dengan system("clear") dan menampilkan dungeon yang cocok dengan level user berurutan berdasarkan index dungeon sambil menunggu 3 detik sebelum menampilkan dungeon selanjutnya.

#### L. Cleanup shared memory setiap system dimatikan
```
void cleanup(int signum) {
    shmctl(shmid, IPC_RMID, NULL);
    printf("Shared memory cleaned up.\n");
    exit(0);
}

```
Function menggunakan shmctl untuk menghapus shared memory dan exit untuk mengakhiri program. 
