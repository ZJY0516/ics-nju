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
void utoa(unsigned int num, char *s, int base)
{
    int i = 0;
    do {
        s[i++] = "0123456789abcdef"[num % base];
        num /= base;
    } while (num);
    s[i] = '\0';
    char temp;
    for (int j = 0; j <= (i - 1) / 2; j++) {
        temp = s[j];
        s[j] = s[i - 1 - j];
        s[i - 1 - j] = temp;
    }
}
static char *append(char *s, char *tmp)
{
    /*append tmp to s and return the pointer to the last character*/
    while (*tmp) {
        *s = *tmp;
        s++;
        tmp++;
    }
    return s;
}

int printf(const char *fmt, ...)
{
    // putstr("in my printf: \n");
    va_list ap;
    va_start(ap, fmt);
    char out[10240];
    int re = vsprintf(out, fmt, ap);
    va_end(ap);
    for (const char *p = out; *p; p++) {
        putch(*p);
    }
    // memset(out, '\0', sizeof(out));
    return re;
}

int vsprintf(char *out, const char *fmt, va_list ap)
{ // more format should be supported
    int num, unum;
    char anything[10240];
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
            case 'u':
                unum = va_arg(ap, unsigned int);
                utoa(unum, tmp, 10);
                s = append(s, tmp);
                break;
            case 's':
                tmp = va_arg(ap, char *);
                putstr("tmp: ");
                putstr(tmp);
                putch('\n');
                s = append(s, tmp);
                break;
            }
            memset(anything, '\0', sizeof(anything));
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
