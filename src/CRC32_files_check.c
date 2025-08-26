#include <dirent.h>
#include <linux/limits.h>
#include <signal.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <time.h>
#include <unistd.h>

struct init_args {
    char folder[PATH_MAX];
    int32_t check_time;
};

struct file_data {
    char path[PATH_MAX];
    FILE *fd;
    uint32_t CRC32;
};

static void print_help(void)
{
    printf("Commands:\n"
           "  Required parameters:\n"
           "    -f(CHECK_FOLDER)       \"/test/\"      Check folder\n"
           "    -t(CHECK_FOLDER_TIME)  \"xxxx\"        Check folder every x seconds\n");
}

#define CRC32_OK 0
#define CRC32_FD_ERROR -1
#define CRC32_INT_ERROR -2
#define CRC32_DATA_ERROR -3

#define READ_DATA_SIZE 1024 * 1024

uint_least32_t crc32_lib(const unsigned char *buf, size_t len, uint_least32_t init);

static int32_t CRC32(FILE *fd, uint32_t *crc32_res, char *read_data)
{
    if (fd == NULL) {
        return CRC32_FD_ERROR;
    }

    if (crc32_res == NULL) {
        return CRC32_INT_ERROR;
    }

    if (read_data == NULL) {
        return CRC32_DATA_ERROR;
    }

    uint32_t crc32_res_local = 0xFFFFFFFF;

    fseek(fd, 0, SEEK_SET);

    int32_t readed_bytes = 0;
    do {
        readed_bytes = fread(read_data, sizeof(char), READ_DATA_SIZE, fd);
        crc32_res_local =
            crc32_lib((const unsigned char *)read_data, readed_bytes, crc32_res_local);
    } while (readed_bytes != 0);

    *crc32_res = crc32_res_local ^ 0xFFFFFFFF;

    return CRC32_OK;
}

static void errmsg(const char *format, ...)
{
    va_list args;

    printf("Error: ");

    va_start(args, format);
    vprintf(format, args);
    va_end(args);

    fflush(stdout);

    exit(EXIT_FAILURE);
}

volatile sig_atomic_t exit_flag = 0;

static void main_catch_function(int32_t signo)
{
    if (signo == SIGSEGV) {
        errmsg("SIGSEGV catched\n");
    }

    if (signo != SIGTERM) {
        return;
    }

    exit_flag = 1;
}

volatile sig_atomic_t sig_flag = 0;

static void main_catch_usr_function(int32_t signo)
{
    if (signo != SIGUSR1) {
        return;
    }

    sig_flag = 1;
}

int32_t main(int32_t argc, char *argv[])
{
    printf("Check files in folder started\n\n");

    if (signal(SIGUSR1, main_catch_usr_function) == SIG_ERR) {
        errmsg("Can't set SIGUSR1 signal handler main\n");
    }

    if (signal(SIGQUIT, main_catch_function) == SIG_ERR) {
        errmsg("Can't set SIGQUIT signal handler main\n");
    }

    if (signal(SIGINT, main_catch_function) == SIG_ERR) {
        errmsg("Can't set SIGINT signal handler main\n");
    }

    if (signal(SIGHUP, main_catch_function) == SIG_ERR) {
        errmsg("Can't set SIGHUP signal handler main\n");
    }

    /*if (signal(SIGSTOP, main_catch_function) == SIG_ERR) {
        errmsg("Can't set SIGSTOP signal handler main\n");
    }*/

    if (signal(SIGCONT, main_catch_function) == SIG_ERR) {
        errmsg("Can't set SIGCONT signal handler main\n");
    }

    if (signal(SIGSEGV, main_catch_function) == SIG_ERR) {
        errmsg("Can't set SIGSEGV signal handler main\n");
    }

    if (signal(SIGTERM, main_catch_function) == SIG_ERR) {
        errmsg("Can't set SIGTERM signal handler main\n");
    }

    struct init_args init_args;
    init_args.folder[0] = 0;
    init_args.check_time = 0;

    //Args
    {
        printf("ENV parameters:\n");
        char *folder_env = NULL;
        folder_env = getenv("CHECK_FOLDER");
        if (folder_env) {
            strcpy(init_args.folder, folder_env);
            printf("  Folder     \"%s\"\n", init_args.folder);
        }
        char *time_env = NULL;
        time_env = getenv("CHECK_FOLDER_TIME");
        if (time_env) {
            sscanf(time_env, "%d", &init_args.check_time);
            printf("  Time       \"%d\"\n", init_args.check_time);
        }

        printf("Launch parameters:\n");
        for (int32_t i = 1; i < argc; i++) {
            if (!strcmp(argv[i], "-f")) {
                if (i != argc - 1) {
                    if (strlen(argv[i + 1]) < PATH_MAX) {
                        strcpy(init_args.folder, argv[i + 1]);
                        printf("  Folder     \"%s\"\n", init_args.folder);
                    }
                    i++;
                }
                continue;
            }
            if (!strcmp(argv[i], "-t")) {
                if (i != argc - 1) {
                    sscanf(argv[i + 1], "%d", &init_args.check_time);
                    printf("  Time       \"%d\"\n", init_args.check_time);
                    i++;
                }
                continue;
            }
            print_help();
            errmsg("Unknown command: %s\n", argv[i]);
        }

        if (init_args.folder[0] == 0) {
            print_help();
            errmsg("The program need correct check folder\n");
        }

        if (init_args.check_time == 0) {
            print_help();
            errmsg("The program need correct check folder time\n");
        }
    }
    //Args

    printf("\n");

    int32_t folder_str_size = 0;
    folder_str_size = strlen(init_args.folder);

    int32_t files_count = 0;

    //Files count in folder
    {
        DIR *dir;
        struct dirent *files;
        dir = opendir(init_args.folder);
        if (!dir) {
            errmsg("Can't open folder\n");
        }

        printf("Files in folder %s\n", init_args.folder);
        while ((files = readdir(dir)) != NULL) {
            if (files->d_type == DT_REG) {
                files_count++;
                printf("%s\n", files->d_name);
            }
        }
        closedir(dir);

        printf("Files count %d in folder %s\n", files_count, init_args.folder);
    }
    //Files count in folder

    printf("\n");

    struct file_data *files_data;
    files_data = (struct file_data *)malloc(sizeof(struct file_data) * files_count);
    if (files_data == NULL) {
        errmsg("No free memory for files_data\n");
    }

    char *read_data;
    read_data = (char *)malloc(sizeof(char) * READ_DATA_SIZE);
    if (read_data == NULL) {
        errmsg("No free memory for read_data\n");
    }

    //Init files_data
    {
        DIR *dir;
        struct dirent *files;
        dir = opendir(init_args.folder);
        if (dir == NULL) {
            errmsg("Can't open folder\n");
        }

        int32_t file_index = 0;

        while ((files = readdir(dir)) != NULL) {
            if (files->d_type != DT_REG) {
                continue;
            }

            if (folder_str_size + sizeof('/') + strlen(files->d_name) + sizeof('\0') > PATH_MAX) {
                errmsg("The file path is too long %s/%s\n", init_args.folder, files->d_name);
            }
            sprintf(files_data[file_index].path, "%s/%s", init_args.folder, files->d_name);

            files_data[file_index].fd = fopen(files_data[file_index].path, "r");
            if (files_data[file_index].fd == NULL) {
                errmsg("Can't open %s\n", files_data[file_index].path);
            }

            int32_t crc32_res = CRC32_OK;
            crc32_res = CRC32(files_data[file_index].fd, &files_data[file_index].CRC32, read_data);
            if (crc32_res != CRC32_OK) {
                errmsg("Can't calc CRC32 for %s\n", files_data[file_index].path);
            }

            printf("CRC32 %X %s\n", files_data[file_index].CRC32, files_data[file_index].path);

            file_index++;
        }

        closedir(dir);
    }
    //Init files_data

    printf("\n");

    //Syslog
    {
        openlog("CRC32-files-check", LOG_PID, LOG_USER);
    }
    //Syslog

    //Files check
    {
        time_t now_start = 0;
        time_t now_end = 0;
        int32_t time_diff = 0;

        while (!exit_flag) {
            if (!time_diff) {
                now_start = time(NULL);
            }
            sleep(init_args.check_time - time_diff);
            now_end = time(NULL);
            time_diff = now_end - now_start;

            if (!sig_flag) {
                if (time_diff < init_args.check_time) {
                    continue;
                } else {
                    time_diff = 0;
                }
            } else {
                sig_flag = 0;
            }

            int32_t cycle_status = 0;

            for (int32_t i = 0; i < files_count; i++) {
                int32_t crc32_res = CRC32_OK;
                uint32_t crc32_tmp;
                crc32_res = CRC32(files_data[i].fd, &crc32_tmp, read_data);
                if (crc32_res != CRC32_OK) {
                    errmsg("Can't calc CRC32 for %s\n", files_data[i].path);
                }

                if (files_data[i].CRC32 != crc32_tmp) {
                    cycle_status++;
                    printf("FAIL %s\n", files_data[i].path);
                    syslog(LOG_INFO, "FAIL %s\n", files_data[i].path);
                }

                files_data[i].CRC32 = crc32_tmp;
            }

            if (cycle_status == 0) {
                printf("OK\n");
                syslog(LOG_INFO, "OK\n");
            }
        }
    }
    //Files check

    printf("Check files in folder stoped\n");

    //Free
    {
        closelog();

        if (signal(SIGUSR1, SIG_DFL) == SIG_ERR) {
            errmsg("Can't set SIGUSR1 signal handler main\n");
        }

        if (signal(SIGQUIT, SIG_DFL) == SIG_ERR) {
            errmsg("Can't set SIGQUIT signal handler main\n");
        }

        if (signal(SIGINT, SIG_DFL) == SIG_ERR) {
            errmsg("Can't set SIGINT signal handler main\n");
        }

        if (signal(SIGHUP, SIG_DFL) == SIG_ERR) {
            errmsg("Can't set SIGHUP signal handler main\n");
        }

        /*if (signal(SIGSTOP, SIG_DFL) == SIG_ERR) {
            errmsg("Can't set SIGSTOP signal handler main\n");
        }*/

        if (signal(SIGCONT, SIG_DFL) == SIG_ERR) {
            errmsg("Can't set SIGCONT signal handler main\n");
        }

        if (signal(SIGSEGV, SIG_DFL) == SIG_ERR) {
            errmsg("Can't set SIGSEGV signal handler main\n");
        }

        if (signal(SIGTERM, SIG_DFL) == SIG_ERR) {
            errmsg("Can't set SIGTERM signal handler main\n");
        }

        for (int32_t i = 0; i < files_count; i++) {
            fclose(files_data[i].fd);
        }

        free(files_data);
        free(read_data);
    }
    //Free

    return EXIT_SUCCESS;
}
