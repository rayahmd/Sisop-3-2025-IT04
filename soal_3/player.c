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