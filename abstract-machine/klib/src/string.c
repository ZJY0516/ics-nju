#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s)
{
    assert(s);
    size_t i = 0;
    while (s[i])
        i++;
    return i;
}

char *strcpy(char *dst, const char *src)
{
    assert(dst && src);
    size_t i = 0;
    do {
        dst[i] = src[i];
    } while (src[i++]);
    return dst;
}

char *strncpy(char *dst, const char *src, size_t n)
{
    assert(dst && src);
    char *start = dst;
    while (n && *src) {
        *dst++ = *src++;
        n--;
    }
    while (n--)
        *dst++ = '\0';
    return start;
}

char *strcat(char *dst, const char *src)
{
    assert(dst && src);
    char *start = dst;
    while (*dst) {
        dst++;
    }
    while (*src) {
        *dst++ = *src++;
    }
    *dst = '\0';
    return start;
}

int strcmp(const char *s1, const char *s2)
{
    assert(s1 && s2);
    size_t i = 0;
    while (s1[i] && s2[i] && s1[i] == s2[i]) {
        i++;
    }
    if (s1[i] == '\0' && s2[i] == '\0') {
        return 0;
    } else if (s1[i] != '\0' && s2[i] != '\0') {
        return s1[i] - s2[i];
    } else {
        return (int)(s1[i] - s2[i]);
    }
}

int strncmp(const char *s1, const char *s2, size_t n)
{
    assert(s1 && s2);
    size_t i;
    for (i = 0; i < n; i++) {
        if (s1[i] != s2[i]) {
            return (int)(s1[i] - s2[i]);
        }
        if (s1[i] == '\0' && s2[i] == '\0') {
            return 0;
        }
    }
    return 0;
}

void *memset(void *s, int c, size_t n)
{
    assert(s);
    char *q = s;
    char *end = q + n;
    for (;;) {
        if (q >= end)
            break;
        *q++ = (char)c;
    }
    return s;
}

void *memmove(void *dst, const void *src, size_t n)
{
    assert(dst && src);
    size_t i;
    if (dst <= src) {
        for (i = 0; i < n; i++) {
            *((char *)dst + i) = *((char *)src + i);
        }
    } else if (dst > src) {
        for (i = n; i > 0; i--) {
            *((char *)dst + i) = *((char *)src + i);
        }
    }
    return dst;
}

void *memcpy(void *out, const void *in, size_t n)
{
    printf("%d\n", (int)n);
    printf("in: %d, out: %d\n", in, out);
    assert(in && out);
    size_t i;
    for (i = 0; i < n; i++) {
        *((char *)out + i) = *((char *)in + i);
        // printf("addr: %d\n", (int)in);
    }
    return out;
}

int memcmp(const void *s1, const void *s2, size_t n)
{
    size_t i;
    for (i = 0; i < n; i++) {
        if (((char *)s1)[i] - ((char *)s2)[i])
            return (((char *)s1)[i] - ((char *)s2)[i]);
    }
    return 0;
}

#endif
