#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

/* integer to alphanumeric
 * convert number to string
 */
static char *itoa(int num, char *s, int base)
{
    assert(base >= 2 && base <= 36);
    assert(s);
    char index[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    unsigned int unum;
    if (num < 0) {
        unum = -num;
    } else {
        unum = num;
    }
    int i = 0, j;
    do {
        s[i++] = index[unum % (unsigned)base];
        unum /= base;
    } while (unum);
    if (num < 0) {
        s[i++] = '-';
    }
    s[i] = '\0';
    char temp;
    for (j = 0; j <= (i - 1) / 2; j++) {
        temp = s[j];
        s[j] = s[i - 1 - j];
        s[i - 1 - j] = temp;
    }
    return s;
}
static char *append(char *s, char *tmp)
{
    /*append tmp to s and return the pointer to the last character*/
    while (*tmp) {
        *s++ = *tmp++;
    }
    return s;
}

int printf(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    char out[1024];
    int re = vsprintf(out, fmt, ap);
    va_end(ap);
    for (const char *p = out; *p; p++) {
        putch(*p);
    }
    memset(out, '\0', sizeof(out));
    return re;
}

int vsprintf(char *out, const char *fmt, va_list ap)
{ // more format should be supported
    int num;
    char anything[128];
    char *tmp = anything;
    char *s = out;
    while (*fmt) {
        if (*fmt != '%') {
            *s++ = *fmt++;
        } else {
            switch (*(++fmt)) {
            case 'd':
                num = va_arg(ap, int);
                itoa(num, tmp, 10);
                s = append(s, tmp);
                break;
            case 's':
                tmp = va_arg(ap, char *);
                s = append(s, tmp);
                break;
            }
            fmt++;
        }
    }
    *s = '\0';
    return strlen(out);
}

int sprintf(char *out, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    int re = vsprintf(out, fmt, ap);
    va_end(ap);
    return re;
}

int snprintf(char *out, size_t n, const char *fmt, ...)
{
    panic("Not implemented");
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap)
{
    panic("Not implemented");
}

#endif
