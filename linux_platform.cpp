#include <time.h>
#include <dirent.h>
#include<errno.h>

bool read_entire_file(Input_buffer *input, char *file_path) {
    s64 file_size;
    FILE *file_handle = fopen(file_path, "rb");

    if (file_handle) {
        fseek(file_handle, 0, SEEK_END);
        file_size = ftell(file_handle);
        input->raw_buffer = (char *)malloc(file_size + 1);

        if (input->raw_buffer) {
            fseek(file_handle, 0, SEEK_SET);
            fread(input->raw_buffer, file_size, 1, file_handle);

            input->count = file_size + 1;
            input->raw_buffer[file_size] = 0;
        } else {
            switch(errno) {
                default: {
                    printf("\nERROR: unhandled error in read_entire_file reading the file, error code: %d\n", (s32)error);
                    break;
                };
            }

            fclose(file_handle);
            input->raw_buffer = NULL;
            return false;
        }

        fclose(file_handle);
    } else {
        switch(errno) {
            case ENOENT: {
                printf("\nERROR: file '%s' not found\n", file_path);
                break;
            };
            case EACCES: {
                printf("\nERROR: file '%s' access denied\n", file_path);
                break;
            };
            default: {
                printf("\nERROR: unhandled error in read_entire_file opening the file, error code: %d\n", (s32)error);
                break;
            };
        }
            
        return false;
    }

    return true;
}

u64 get_time_microseconds() {
    clock_t ticks = clock();
    u64 result = ((ticks * 1000000) / CLOCKS_PER_SEC);
    return result;
}

u64 get_time_milliseconds() {
    return get_time_microseconds() / 1000;
}

u64 get_time_seconds() {
    return get_time_microseconds() / 1000000;
}

bool directory_exists(char *path) {
    if (path == NULL) return false;

    DIR *file_handle = opendir((char *)path);

    if (file_handle != NULL) {
        closedir(file_handle);
        return true;
    }

    return false;
}

u32 has_similar_paths(char *path, char* wrong, char results[10][256]) {
    u32 similar_count = 0;
    u32 lenght = strlen((char *)path);

    if (lenght >= ((FOLDERS_COUNT * FOLDERS_NAME_LENGTH) - 2)) return 0;

    struct dirent *find_data;
    DIR *find_handler = opendir((char *)path);

    if (find_handler != NULL){
        find_data = readdir(find_handler);

        while (find_data != NULL){
            f32 s = similarity(wrong, (char *)find_data->d_name);

            if (s >= 0.7) {
                strcpy((char *)results[similar_count], find_data->d_name);
                similar_count++;
            }

            if (similar_count >= 9) break;
            find_data = readdir(find_handler);
        }

        closedir(find_handler);
    }

    return similar_count;
}

void setup_console(void) {
    // https://solarianprogrammer.com/2019/04/08/c-programming-ansi-escape-codes-windows-macos-linux-terminals/
}

void restore_console(void) {
    // https://solarianprogrammer.com/2019/04/08/c-programming-ansi-escape-codes-windows-macos-linux-terminals/
    // Reset colors
    printf("\x1b[0m");
}
