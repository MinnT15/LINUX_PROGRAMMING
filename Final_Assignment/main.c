#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
#include <errno.h>

#define FIFO_PATH "logFifo"  
#define LOG_FILE "gateway.log"  
#define NUM_SENSORS 3  

typedef struct {
    int sensor_id;
    double temperature;
    char log_message[256];
} SensorData;

SensorData sensors[NUM_SENSORS];  
pthread_mutex_t shared_data_mutex;
pthread_mutex_t log_mutex;
int sequence_number = 1;  

// Hàm ghi nhật ký vào FIFO
void log_event(const char* message) {
    pthread_mutex_lock(&log_mutex);

  
    int fd = open(FIFO_PATH, O_WRONLY | O_NONBLOCK);
    if (fd != -1) {
        write(fd, message, strlen(message) + 1);
        close(fd);
    }

    pthread_mutex_unlock(&log_mutex);
}

// Hàm của Process Ghi Nhật Ký (Log Process)
void* log_process(void* arg) {
    char buffer[512];
    FILE* log_file = fopen(LOG_FILE, "a");
    if (log_file == NULL) {
        perror("Failed to open log file");
        exit(1);
    }

    int fd = open(FIFO_PATH, O_RDONLY);
    if (fd == -1) {
        perror("Failed to open FIFO for reading");
        exit(1);
    }

    printf("Log process started. Waiting for log events...\n");

    while (1) {
        ssize_t bytes_read = read(fd, buffer, sizeof(buffer));
        if (bytes_read > 0) {
            buffer[bytes_read] = '\0';  
            time_t now = time(NULL);
            struct tm* local_time = localtime(&now);
            fprintf(log_file, "%d %04d-%02d-%02d %02d:%02d:%02d %s\n",
                    sequence_number++,
                    local_time->tm_year + 1900,
                    local_time->tm_mon + 1,
                    local_time->tm_mday,
                    local_time->tm_hour,
                    local_time->tm_min,
                    local_time->tm_sec,
                    buffer);
            fflush(log_file);
            printf("[Log Process] Logged: %s\n", buffer);
        }
        sleep(1);  
    }

    fclose(log_file);
    close(fd);
    return NULL;
}

//  Thread Quản Lý Kết Nối
void* connection_manager(void* arg) {
    int sensor_index = *(int*)arg;
    free(arg);

    while (1) {
        pthread_mutex_lock(&shared_data_mutex);
        
        // Giả lập kết nối cảm biến
        sensors[sensor_index].temperature = 20.0 + sensor_index * 2;  // Giả lập nhiệt độ
        snprintf(sensors[sensor_index].log_message, sizeof(sensors[sensor_index].log_message),
                 "Sensor %d connected with temperature %.2f",
                 sensors[sensor_index].sensor_id, sensors[sensor_index].temperature);

        printf("[Connection Manager] %s\n", sensors[sensor_index].log_message);
        
        // Ghi nhật ký khi cảm biến kết nối
        log_event(sensors[sensor_index].log_message);

        
        if (sensor_index == 0) {  // Giả sử cảm biến ID 1 kết nối
            snprintf(sensors[sensor_index].log_message, sizeof(sensors[sensor_index].log_message),
                     "Sensor %d has opened a new connection", sensors[sensor_index].sensor_id);
            log_event(sensors[sensor_index].log_message);
        }

        pthread_mutex_unlock(&shared_data_mutex);
        sleep(2);  
    }
    return NULL;
}

// Thread Quản Lý Dữ Liệu
void* data_manager(void* arg) {
    while (1) {
        pthread_mutex_lock(&shared_data_mutex);
        for (int i = 0; i < NUM_SENSORS; i++) {
            // Giả lập thay đổi nhiệt độ
            sensors[i].temperature = 20.0 + (rand() % 20);

            
            if (sensors[i].temperature < 20.0) {
                snprintf(sensors[i].log_message, sizeof(sensors[i].log_message),
                         "Sensor %d: Too cold (%.2f°C)", sensors[i].sensor_id, sensors[i].temperature);
            } else if (sensors[i].temperature > 30.0) {
                snprintf(sensors[i].log_message, sizeof(sensors[i].log_message),
                         "Sensor %d: Too hot (%.2f°C)", sensors[i].sensor_id, sensors[i].temperature);
            } else {
                snprintf(sensors[i].log_message, sizeof(sensors[i].log_message),
                         "Sensor %d: Normal temperature (%.2f°C)", sensors[i].sensor_id, sensors[i].temperature);
            }

            printf("[Data Manager] %s\n", sensors[i].log_message);
            log_event(sensors[i].log_message);
        }
        pthread_mutex_unlock(&shared_data_mutex);
        sleep(3);  
    }
    return NULL;
}

// Thread Quản Lý Lưu Trữ
void* storage_manager(void* arg) {
    while (1) {
        pthread_mutex_lock(&shared_data_mutex);
        for (int i = 0; i < NUM_SENSORS; i++) {
            snprintf(sensors[i].log_message, sizeof(sensors[i].log_message),
                     "Storing data: Sensor %d, Temperature %.2f",
                     sensors[i].sensor_id, sensors[i].temperature);
            printf("[Storage Manager] %s\n", sensors[i].log_message);
            log_event(sensors[i].log_message);
        }
        pthread_mutex_unlock(&shared_data_mutex);
        sleep(5);  
    }
    return NULL;
}

int main() {
    pthread_t conn_threads[NUM_SENSORS], data_thread, storage_thread;

  
    if (mkfifo(FIFO_PATH, 0666) == -1 && errno != EEXIST) {
        perror("Failed to create FIFO");
        return 1;
    }

    // REQ 1: Tạo Process ghi nhật ký
    pid_t pid = fork();
    if (pid == -1) {
        perror("Failed to fork process");
        return 1;
    }
    if (pid == 0) {
        log_process(NULL);
        exit(0);
    }

    // REQ 2: Tạo 3 Thread cho quá trình chính
    pthread_mutex_init(&shared_data_mutex, NULL);
    pthread_mutex_init(&log_mutex, NULL);

    // Tạo Thread kết nối cảm biến
    for (int i = 0; i < NUM_SENSORS; i++) {
        sensors[i].sensor_id = i + 1;
    }

  
    for (int i = 0; i < NUM_SENSORS; i++) {
        int* sensor_index = malloc(sizeof(int));
        *sensor_index = i;
        if (pthread_create(&conn_threads[i], NULL, connection_manager, sensor_index) != 0) {
            perror("Failed to create connection manager thread");
            return 1;
        }
    }

    // Tạo Thread Quản lý Dữ Liệu
    if (pthread_create(&data_thread, NULL, data_manager, NULL) != 0) {
        perror("Failed to create data manager thread");
        return 1;
    }

    // Tạo Thread Quản lý Lưu Trữ
    if (pthread_create(&storage_thread, NULL, storage_manager, NULL) != 0) {
        perror("Failed to create storage manager thread");
        return 1;
    }

    for (int i = 0; i < NUM_SENSORS; i++) {
        pthread_join(conn_threads[i], NULL);
    }
    pthread_join(data_thread, NULL);
    pthread_join(storage_thread, NULL);

 
    pthread_mutex_destroy(&shared_data_mutex);
    pthread_mutex_destroy(&log_mutex);

    return 0;
}
