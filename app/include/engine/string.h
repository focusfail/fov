#ifndef __ENGINE_STRING_H__
#define __ENGINE_STRING_H__

/// @brief Duplicate a string
/// @param string
/// @return Pointer to duplicated string
extern char* e_strdup(const char* string);

/// @brief Duplicate a string according to length
/// @param string
/// @param length The length of the string
/// @return Pointer to the duplicated string
extern char* e_strdupl(const char* string, size_t length);

#ifdef STR_IMPL

#include <string.h>

char* e_strdup(const char* string)
{
    if (!string) return NULL;

    char* duplicate = malloc(strlen(string) + 1);
    if (!duplicate) return NULL;

    strcpy(duplicate, string);
    return duplicate;
}

char* e_strdupl(const char* string, size_t length)
{
    if (!string) return NULL;

    char* duplicate = malloc(length + 1);
    if (!duplicate) return NULL;

    memcpy(duplicate, string, length);
    duplicate[length] = '\0';

    return duplicate;
}

#endif // STR_IMPL
#endif // __ENGINE_STRING_H__