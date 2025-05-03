#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
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
int total_orders;
sem_t *sem;

void log_delivery(const char *agent, const char *nama, const char *alamat) {
    FILE *log_file = fopen("delivery.log", "a");
    if (!log_file) {
        perror("Error opening log file");
        return;
    }
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    fprintf(log_file, "[%02d/%02d/%04d %02d:%02d:%02d] [%s] Express package delivered to %s in %s \n",
            t->tm_mday, t->tm_mon + 1, t->tm_year + 1900,
            t->tm_hour, t->tm_min, t->tm_sec,
            agent, nama, alamat);
    fclose(log_file);
}

void *express_agent(void *arg) {
    char *agent_name = (char *)arg;

    while (1) {
        sem_wait(sem);
        for (int i = 0; i < total_orders; i++) {
            if (strcmp(orders[i].jenis, "Express") == 0 && orders[i].delivered == 0) {
                orders[i].delivered = 1;
                strcpy(orders[i].agent, agent_name);
                log_delivery(agent_name, orders[i].nama, orders[i].alamat);
                break;
            }
        }
        sem_post(sem);
        sleep(1);
    }

    return NULL;
}

int main() {
    int shm_fd = shm_open(SHARED_MEM_NAME, O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("Error opening shared memory in delivery_agent");
        exit(EXIT_FAILURE);
    }

    orders = mmap(0, sizeof(Order) * MAX_ORDERS, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (orders == MAP_FAILED) {
        perror("Error mapping shared memory in delivery_agent");
        exit(EXIT_FAILURE);
    }

    sem = sem_open("/delivery_order", 0);
    if (sem == SEM_FAILED) {
        perror("Error opening semaphore in delivery_agent");
        exit(EXIT_FAILURE);
    }

    sem_wait(sem); 
    total_orders = 0;
    while (orders[total_orders].nama[0] != '\0' && total_orders < MAX_ORDERS) {
        total_orders++;
    }
    sem_post(sem);

    pthread_t agent_a, agent_b, agent_c;
    pthread_create(&agent_a, NULL, express_agent, "AGENT A");
    pthread_create(&agent_b, NULL, express_agent, "AGENT B");
    pthread_create(&agent_c, NULL, express_agent, "AGENT C");

    pthread_join(agent_a, NULL);
    pthread_join(agent_b, NULL);
    pthread_join(agent_c, NULL);

    munmap(orders, sizeof(Order) * MAX_ORDERS);
    close(shm_fd);
    sem_close(sem);
    shm_unlink(SHARED_MEM_NAME); 
    sem_unlink("/delivery_order"); 
    return 0;
}
