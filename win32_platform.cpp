#include<windows.h>

bool read_entire_file(Input_buffer *input, char *file_path) {
    HANDLE file_handle = CreateFileA(
        (char *)file_path,
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (file_handle != INVALID_HANDLE_VALUE) {
        LARGE_INTEGER  size = {};
        GetFileSizeEx(file_handle, &size);

        input->count = (u64)size.LowPart;
        input->raw_buffer = (char *)malloc(sizeof(char) * input->count);

        if (ReadFile(file_handle, (void *)input->raw_buffer, (DWORD)input->count, NULL, NULL)) {
        } else {
            DWORD error = GetLastError();

            switch(error) {
                case ERROR_INSUFFICIENT_BUFFER: {
                    printf("\nERROR: buffer too small to hold the entire file, current file set to: %lld\n", input->count);
                    break;
                };
                default: {
                    printf("\nERROR: unhandled error in read_entire_file reading the file, error code: %d\n", (s32)error);
                    break;
                };
            }

            CloseHandle(file_handle);
            return false;
        }
        
        CloseHandle(file_handle);
    } else {
        DWORD error = GetLastError();

        switch(error) {
            case ERROR_FILE_NOT_FOUND: {
                printf("\nERROR: file '%s' not found\n", file_path);
                break;
            };
            case ERROR_ACCESS_DENIED: {
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
    LARGE_INTEGER ticks, frequency;
    QueryPerformanceCounter(&ticks);
    QueryPerformanceFrequency(&frequency);

    u64 result = ((ticks.QuadPart * 1000000) / frequency.QuadPart);
    return result;
}

u64 get_time_milliseconds() {
    return get_time_microseconds() / 1000;
}


u64 get_time_seconds() {
    return get_time_microseconds() / 1000000;
}

bool directory_exists(char *path) {
    DWORD ftyp = GetFileAttributesA((char *)path);

    if (ftyp == INVALID_FILE_ATTRIBUTES) return false;

    if (ftyp & FILE_ATTRIBUTE_DIRECTORY) return true;

    return false;
}

u32 has_similar_paths(char *path, char* wrong, char results[10][256]) {
    u32 similar_count = 0;
    u32 lenght = strlen((char *)path);

    if (lenght >= ((FOLDERS_COUNT * FOLDERS_NAME_LENGTH) - 2)) return 0;

    path[lenght] = '*';
    path[lenght + 1] = 0;

    WIN32_FIND_DATA find_data;
    HANDLE find_handler = FindFirstFileEx((LPCSTR)path, FindExInfoStandard, &find_data, FindExSearchNameMatch, NULL, 0);

    path[lenght] = 0;

    while(FindNextFileA(find_handler, &find_data)) {
        f32 s = similarity(wrong, (char *)find_data.cFileName);

        if (s >= 0.7) {
            strcpy((char *)results[similar_count], (char *)find_data.cFileName);
            similar_count++;
        }

        if (similar_count >= 9) break;
    }

    FindClose(find_handler);

    return similar_count;
}

#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING  0x0004
#endif

static HANDLE stdoutHandle;
static DWORD outModeInit;

void setup_console(void) {
    // https://solarianprogrammer.com/2019/04/08/c-programming-ansi-escape-codes-windows-macos-linux-terminals/
    // Enable ANSI escape codes
    DWORD outMode = 0;
    stdoutHandle = GetStdHandle(STD_OUTPUT_HANDLE);

    if(stdoutHandle == INVALID_HANDLE_VALUE) {
        exit(GetLastError());
    }

    if(!GetConsoleMode(stdoutHandle, &outMode)) {
        exit(GetLastError());
    }

    outModeInit = outMode;
    outMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;

    if(!SetConsoleMode(stdoutHandle, outMode)) {
        exit(GetLastError());
    }   
}

void restore_console(void) {
    // https://solarianprogrammer.com/2019/04/08/c-programming-ansi-escape-codes-windows-macos-linux-terminals/
    // Reset console mode
    printf("\x1b[0m");   

    if(!SetConsoleMode(stdoutHandle, outModeInit)) {
       exit(GetLastError());
    }
}
