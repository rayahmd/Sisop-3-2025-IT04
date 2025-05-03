#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define SERVER_IP "127.0.0.1"
#define PORT 8080
#define BUFFER_SIZE 1024

void display_menu() {
    printf("\n=== Sistem Konversi Gambar ===\n");
    printf("1. Unggah File Teks\n");
    printf("2. Unduh File JPEG\n");
    printf("3. Keluar\n");
    printf("Pilih (1-3): ");
}

int connect_to_server() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr = {AF_INET, htons(PORT)};
    inet_pton(AF_INET, SERVER_IP, &addr.sin_addr);
    if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        printf("ERROR: Gagal connect ke server\n");
        return -1;
    }
    return sock;
}

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