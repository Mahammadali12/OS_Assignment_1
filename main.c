#include <stdio.h>
#include <stdlib.h>
#include <sys/utsname.h>
#include <time.h>
#include <sys/statvfs.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>

#define MAX_PATH 4096
#define CONFIG_FILE ".sysinfo.conf"

// Structure to hold application settings
typedef struct {
    char last_visited_path[MAX_PATH];
    int show_hidden_files;
} AppSettings;

// Function to load settings
AppSettings load_settings() {
    AppSettings settings = {.show_hidden_files = 0};
    char config_path[MAX_PATH];
    snprintf(config_path, sizeof(config_path), "%s/%s", getenv("HOME"), CONFIG_FILE);
    
    FILE *fp = fopen(config_path, "r");
    if (fp != NULL) {
        fscanf(fp, "%s\n%d", settings.last_visited_path, &settings.show_hidden_files);
        fclose(fp);
    } else {
        getcwd(settings.last_visited_path, MAX_PATH);
    }
    return settings;
}

// Function to save settings
void save_settings(AppSettings *settings) {
    char config_path[MAX_PATH];
    snprintf(config_path, sizeof(config_path), "%s/%s", getenv("HOME"), CONFIG_FILE);
    
    FILE *fp = fopen(config_path, "w");
    if (fp != NULL) {
        fprintf(fp, "%s\n%d", settings->last_visited_path, settings->show_hidden_files);
        fclose(fp);
    }
}

// Function to display system information
void display_system_info() {
    struct utsname sys_info;
    if (uname(&sys_info) == -1) {
        perror("Error getting system information");
        return;
    }

    printf("\n=== System Information ===\n");
    printf("System name: %s\n", sys_info.sysname);
    printf("Node name: %s\n", sys_info.nodename);
    printf("Release: %s\n", sys_info.release);
    printf("Version: %s\n", sys_info.version);
    printf("Machine: %s\n", sys_info.machine);

    // Display current time
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    char time_str[50];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", tm_info);
    printf("\nCurrent time: %s\n", time_str);
}

// Function to display drive information
void display_drive_info(const char *path) {
    struct statvfs stat;
    if (statvfs(path, &stat) == 0) {
        printf("\n=== Drive Information for %s ===\n", path);
        printf("Total size: %.2f GB\n", (float)(stat.f_blocks * stat.f_frsize) / (1024 * 1024 * 1024));
        printf("Free space: %.2f GB\n", (float)(stat.f_bfree * stat.f_frsize) / (1024 * 1024 * 1024));
        printf("Available space: %.2f GB\n", (float)(stat.f_bavail * stat.f_frsize) / (1024 * 1024 * 1024));
    }
}

// Function to display directory contents
void list_directory(const char *path, int show_hidden) {
    DIR *dir;
    struct dirent *entry;
    struct stat file_stat;
    char full_path[MAX_PATH];

    dir = opendir(path);
    if (dir == NULL) {
        perror("Error opening directory");
        return;
    }

    printf("\n=== Directory Contents of %s ===\n", path);
    printf("%-40s %-10s %-10s\n", "Name", "Size (B)", "Type");
    printf("----------------------------------------\n");

    while ((entry = readdir(dir)) != NULL) {
        if (!show_hidden && entry->d_name[0] == '.') {
            continue;
        }

        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);
        if (stat(full_path, &file_stat) == 0) {
            printf("%-40s %-10ld %-10s\n", 
                entry->d_name, 
                file_stat.st_size,
                S_ISDIR(file_stat.st_mode) ? "Directory" : "File");
        }
    }

    closedir(dir);
}

int main() {
    AppSettings settings = load_settings();
    char current_path[MAX_PATH];
    strcpy(current_path, settings.last_visited_path);
    int running = 1;

    while (running) {
        printf("\n=== System Information Tool ===\n");
        printf("1. Display System Information\n");
        printf("2. Display Drive Information\n");
        printf("3. List Directory Contents\n");
        printf("4. Change Directory\n");
        printf("5. Toggle Hidden Files (%s)\n", settings.show_hidden_files ? "ON" : "OFF");
        printf("6. Exit\n");
        printf("Choose an option: ");

        int choice;
        scanf("%d", &choice);
        getchar(); // Consume newline

        switch (choice) {
            case 1:
                display_system_info();
                break;
            case 2:
                display_drive_info(current_path);
                break;
            case 3:
                list_directory(current_path, settings.show_hidden_files);
                break;
            case 4:
                printf("Enter new path: ");
                fgets(current_path, MAX_PATH, stdin);
                current_path[strcspn(current_path, "\n")] = 0; // Remove newline
                strcpy(settings.last_visited_path, current_path);
                save_settings(&settings);
                break;
            case 5:
                settings.show_hidden_files = !settings.show_hidden_files;
                save_settings(&settings);
                break;
            case 6:
                running = 0;
                break;
            default:
                printf("Invalid option!\n");
        }
    }

    return 0;
}