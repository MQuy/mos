#include "string.h"
#include <stdint.h>

char tbuf[32];
char bchars[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

void itoa(unsigned i, unsigned base, char *buf)
{
    int pos = 0;
    int opos = 0;
    int top = 0;

    if (i == 0 || base > 16)
    {
        buf[0] = '0';
        buf[1] = '\0';
        return;
    }

    while (i != 0)
    {
        tbuf[pos] = bchars[i % base];
        pos++;
        i /= base;
    }
    top = pos--;
    for (opos = 0; opos < top; pos--, opos++)
    {
        buf[opos] = tbuf[pos];
    }
    buf[opos] = 0;
}

void itoa_s(int i, unsigned base, char *buf)
{
    if (base > 16)
        return;
    if (i < 0)
    {
        *buf++ = '-';
        i *= -1;
    }
    itoa(i, base, buf);
}

/* K&R */
void reverse(char s[])
{
    int c, i, j;
    for (i = 0, j = strlen(s) - 1; i < j; i++, j--)
    {
        c = s[i];
        s[i] = s[j];
        s[j] = c;
    }
}

/* K&R */
int strlen(char s[])
{
    int i = 0;
    while (s[i] != '\0')
        ++i;
    return i;
}

void append(char s[], char n)
{
    int len = strlen(s);
    s[len] = n;
    s[len + 1] = '\0';
}

void backspace(char s[])
{
    int len = strlen(s);
    s[len - 1] = '\0';
}

/* K&R 
 * Returns <0 if s1<s2, 0 if s1==s2, >0 if s1>s2 */
int strcmp(char s1[], char s2[])
{
    int i;
    for (i = 0; s1[i] == s2[i]; i++)
    {
        if (s1[i] == '\0')
            return 0;
    }
    return s1[i] - s2[i];
}