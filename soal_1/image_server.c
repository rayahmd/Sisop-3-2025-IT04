#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <syslog.h>
#include <jpeglib.h>
#include <time.h>

#define DATABASE_DIR "server/database/"
#define PORT 8080
#define BUFFER_SIZE 2048


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

void reverse(char *text, char *output){
    int len = strlen(text);
    for(int i = 0; i<len; i++) output[i] = text[len - 1 - i];
    output[len] = '\0';
}

int hexascii(char *hex, char *output){
    int len = strlen(hex);
    if (len % 2 != 0) return -1;
    for (int i = 0; i < len / 2; i++){
        sscanf(hex + 2 * i, "%2hhx", &output[i]);
    }
    output[len / 2] = '\0';
    return 0;
}

void create_jpeg(char *data, char *filename) {
    FILE *outfile = fopen(filename, "wb");
    if (!outfile) return;
    fprintf(outfile, "JPEG placeholder: %s", data); 
    fclose(outfile);
}

void client(int client_sock) {
    char buffer[BUFFER_SIZE];
    int n;
    while ((n = read(client_sock, buffer, BUFFER_SIZE - 1)) > 0){
        buffer[n] = '\0';
        char cmd[16], filename[256], content[BUFFER_SIZE];
        sscanf(buffer, "%s %s %[^\n]", cmd, filename, content);

        if(strcmp(cmd, "UPLOAD_TEXT") == 0){
            char reversed[BUFFER_SIZE], decoded[BUFFER_SIZE];
            reverse(content, reversed);
            if(hexascii(reversed, decoded) < 0){
                write(client_sock, "Invalid HEX", 12);
                syslog(LOG_ERR, "Invalid hex for file: %s", filename);
                continue;
            }
            time_t now = time(NULL);
            char image_path[512];
            snprintf(image_path, 512, "%s%ld.jpeg", DATABASE_DIR, now);
            create_jpeg(decoded, image_path);
            write(client_sock, "Succesfully processed!", 23);
            syslog(LOG_INFO, "Processed file: %s", filename);
        }
        else if(strcmp(cmd, "DOWNLOAD_JPEG") == 0){
            char filepath[512];
            snprintf(filepath, 512, "%s%s", DATABASE_DIR, filename);
            FILE *file = fopen(filepath, "rb");
            if (!file) {
                write(client_sock, "ERROR: File not found", 22);
                syslog(LOG_ERR, "File not found: %s", filename);
                continue;
            }
            fseek(file, 0, SEEK_END);
            long fsize = ftell(file);
            fseek(file, 0, SEEK_SET);
            char *data = malloc(fsize);
            fread(data, 1, fsize, file);
            fclose(file);
            write(client_sock, data, fsize);
            free(data);
            syslog(LOG_INFO, "Sent %s", filename);
        }
    }
    close(client_sock);
}

int main() {
    daemonize();
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr = {AF_INET, htons(PORT), INADDR_ANY};
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));
    bind(server_fd, (struct sockaddr *)&addr, sizeof(addr));
    listen(server_fd, 10);
    syslog(LOG_INFO, "Server started");

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t len = sizeof(client_addr);
        int client_sock = accept(server_fd, (struct sockaddr *)&client_addr, &len);
        if (client_sock >= 0) client(client_sock);
    }
    close(server_fd);
    closelog();
    return 0;
}










