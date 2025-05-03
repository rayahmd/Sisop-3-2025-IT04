#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <semaphore.h>

#define SHARED_MEM_NAME "/delivery_order"
#define MAX_ORDERS 100

typedef struct {
    char nama[50];
    char alamat[100];
    char jenis[10];
    int delivered;
    char agent[20];
} Order;

Order *orders;
int total_orders = 0;
sem_t *sem;

void log_delivery(const char *agent, const char *nama, const char *alamat) {
    FILE *log_file = fopen("delivery.log", "a");
    if (!log_file) {
        perror("Error opening log file");
        return;
    }
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    fprintf(log_file, "[%02d/%02d/%04d %02d:%02d:%02d] [%s] Reguler package delivered to %s in %s\n",
            t->tm_mday, t->tm_mon + 1, t->tm_year + 1900,
            t->tm_hour, t->tm_min, t->tm_sec,
            agent, nama, alamat);
    fclose(log_file);
}

void dispatcher_deliver(const char *nama) {
    sem_wait(sem);
    for (int i = 0; i < total_orders; i++) {
        if (strcmp(orders[i].nama, nama) == 0 && strcmp(orders[i].jenis, "Reguler") == 0 && orders[i].delivered == 0) {
            orders[i].delivered = 1;
            log_delivery("Dinda", orders[i].nama, orders[i].alamat);
            strcpy(orders[i].agent, "Dinda");
            sem_post(sem);
            return;
        }
    }
    sem_post(sem);
    printf("Order not found or already delivered\n");
}

void dispatcher_status(const char *nama) {
    sem_wait(sem);
    for (int i = 0; i < total_orders; i++) {
        if (strcmp(orders[i].nama, nama) == 0) {
            printf("Status for %s: %s\n", nama, orders[i].agent);
            sem_post(sem);
            return;
        }
    }
    sem_post(sem);
    printf("Order not found\n");
}

void dispatcher_list() {
    sem_wait(sem);
    for (int i = 0; i < total_orders; i++) {
        printf("%s - %s\n", orders[i].nama, orders[i].agent);
    }
    sem_post(sem);
}

int main(int argc, char *argv[]) {
    int shm_fd = shm_open(SHARED_MEM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("Error opening shared memory in dispatcher");
        exit(EXIT_FAILURE);
    }
    ftruncate(shm_fd, sizeof(Order) * MAX_ORDERS);

    orders = mmap(0, sizeof(Order) * MAX_ORDERS, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (orders == MAP_FAILED) {
        perror("Error mapping shared memory in dispatcher");
        exit(EXIT_FAILURE);
    }

    sem = sem_open("/delivery_order", O_CREAT, 0666, 0); 
    if (sem == SEM_FAILED) {
        perror("Error opening semaphore in dispatcher");
        exit(EXIT_FAILURE);
    }

    FILE *file = fopen("delivery_order.csv", "r");
    if (!file) {
        perror("Error opening CSV file in dispatcher");
        exit(EXIT_FAILURE);
    }

    char line[256];
    total_orders = 0;

    while (fgets(line, sizeof(line), file)) {
        if (total_orders >= MAX_ORDERS) break;
        sscanf(line, "%49[^,],%99[^,],%9s",
               orders[total_orders].nama,
               orders[total_orders].alamat,
               orders[total_orders].jenis);
        orders[total_orders].delivered = 0;
        strcpy(orders[total_orders].agent, "Pending");
        total_orders++;
    }
    fclose(file);

    sem_post(sem); 

    if (argc == 2 && strcmp(argv[1], "-list") == 0) {
        dispatcher_list();
    } else if (argc == 3 && strcmp(argv[1], "-status") == 0) {
        dispatcher_status(argv[2]);
    } else if (argc == 3 && strcmp(argv[1], "-deliver") == 0) {
        dispatcher_deliver(argv[2]);
    } else {
        printf("Invalid command\n");
    }

    munmap(orders, sizeof(Order) * MAX_ORDERS);
    close(shm_fd);
    sem_close(sem);
    return 0;
}
