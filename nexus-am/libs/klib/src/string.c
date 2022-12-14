#include "klib.h"

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
    int i;
    for (i = 0; s[i] != '\0'; i++) {}
    return i;
}

char *strcpy(char* dst,const char* src) {
    int i;
    for (i = 0; src[i] != '\0'; i++) {
        dst[i] = src[i];
    }
    dst[i] = '\0';
    return dst;
}

char* strncpy(char* dst, const char* src, size_t n) {
    int i;
    if (n > strlen(src)) n = strlen(src);
    for (i = 0; i < n; i++) {
        dst[i] = src[i];
    }
    dst[i] = '\0';
    return dst;
}

char* strcat(char* dst, const char* src) {
    int i = 0;
    while (dst[i] != '\0') i++;
    while (*src != '\0') {
        dst[i++] = *src++;
    }
    dst[i] = '\0';
    return dst;
}

int strcmp(const char* s1, const char* s2) {
    int i = 0;
    while (s1[i] != '\0' && s2[i] != '\0') {
        if (s1[i] < s2[i]) {
            return -1;
        }
        else if (s1[i] > s2[i]) {
            return 1;
        }
        i++;
    }
    if (s1[i] == '\0' && s2[i] == '\0') {
        return 0;
    }
    else if (s1[i] == '\0') {
        return -1;
    }
    else {
        return 1;
    }
}

int strncmp(const char* s1, const char* s2, size_t n) {
    int i = 0;
    while (n > 0 && s1[i] != '\0' && s2[i] != '\0') {
        if (s1[i] < s2[i]) {
            return -1;
        }
        else if (s1[i] > s2[i]) {
            return 1;
        }
        i++;
        n--;
    }
    if (n != 0) {
        if (s1[i] == '\0' && s2[i] == '\0') {
            return 0;
        }
        else if (s1[i] == '\0') {
            return -1;
        }
        else {
            return 1;
        }
    }
    else {
        return 0;
    }
}

void* memset(void* v,int c,size_t n) {
    const unsigned char uc = c;
    unsigned char* su = v;
    while (n--) {
        *su = uc;
        su++;
    }
    return v;
}

void* memcpy(void* out, const void* in, size_t n) {
    if (out == NULL || in == NULL) {
        return NULL;
    }
    void *ret = out;
    while (n--) {
        *(char*)out = *(char*)in;
        out = (char*)out + 1;
        in = (char*)in + 1;
    }
    return ret;
}

int memcmp(const void* s1, const void* s2, size_t n){
    if (!n) return 0;
    while (--n && *(char*)s1 == *(char*)s2) {
        s1 = (char*)s1 + 1;
        s2 = (char*)s2 + 1;
    }
    return (*(char*)s1 - *(char*)s2);
}

#endif
