#include "engine/file.h"

#include "log.h"
#include <stdio.h>
#include <stdlib.h>

char* file_read_bytes(const char* filepath, long* bytes_read)
{
    FILE* file = fopen(filepath, "rb");
    if (!file) {
        log_error("Failed to open file: %s", filepath);
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    *bytes_read = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* buffer = (char*)malloc(*bytes_read + 1);
    if (!buffer) {
        log_error("Failed to allocate memory for file: %s", filepath);
        fclose(file);
        return NULL;
    }

    fread(buffer, 1, *bytes_read, file);
    buffer[*bytes_read] = '\0';

    fclose(file);
    return buffer;
}