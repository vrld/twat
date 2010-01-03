/* This is a project for a seminar at the Hochschule fuer Gestaltung Karlsruhe.
 * Feel free to use it within the bounds of the following license:
 *
 * Copyright (c) 2010 Matthias Richter
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to ect to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Except as contained in this notice, the name(s) of the above
 * copyright holders shall not be used in advertising or otherwise
 * to promote the sale, use or other dealings in this Software
 * without prior written authorization.
 */
#include <string.h>
#include <stdlib.h>
#include "stringutils.h"

char* str_replace(const char* restrict src, const char* restrict s, const char* restrict r)
{
    const char *p, *q = src;
    char* ret;
    int n = 0;
    size_t lens = strlen(s), lenr = strlen(r), len, pos;

    for (p = src; (p = strstr(p, s)); p += lens)
        ++n;

    len = strlen(src) + 1 + n * (lenr - lens);
    ret = malloc(len);
    for (p = strstr(src, s); p; p = strstr(q, s)) {
        pos = (size_t)(p - q);
        strncpy(ret, q, pos);

        ret += pos;
        strncpy(ret, r, lenr);
        ret += lenr;

        q += pos + lens; /* advance q to the end of copied and replaced section */
    }
    strcpy(ret, q);

    /* rewind string to point at beginning */
    ret -= len - 1 - strlen(q);

    return ret;
}

char* str_replace_all(const char* restrict src, const char** restrict sr)
{
    char *ret = NULL;
    char *temp = NULL;
    const char *s, *r, *cur = src;
    const char **q;

    for (q = sr; *q && *(q+1); q += 2) {
        s = *q;
        r = *(q+1);
        temp = str_replace(cur, s, r);
        if (ret) free(ret);
        cur = ret = temp;
    }

    return ret;
}

//char* str_replace_char(const char* restrict src, char s, const char* restrict r)
//{
//    const char* p;
//    const char* pr;
//    char* ret = NULL;
//    int n = 0;
//    size_t len;
//
//    for (p = src; *p; ++p)
//        if (s == *p) ++n;
//
//    if (1 == strlen(r)) 
//    {
//        ret = malloc(strlen(src) + 1);
//        char_replace(ret, s, r[0]);
//        return ret;
//    }
//
//    /* reserve space for original string with replaced 's' */
//    len = strlen(src) + 1 + n * strlen(r) - n;
//    ret = malloc(len);
//
//    /* copy src to ret, replacing every s with r */ 
//    for (p = src; *p; ++p)
//    {
//        if (s == *p) {
//            for (pr = r; (*ret = *pr); ++pr, ++ret)
//                /* copying takes place above */;
//        } else {
//            *ret = *p;
//            ++ret;
//        }
//    }
//
//    /* rewind string */
//    ret -= len;
//    ++ret; /* null byte */
//
//    return ret;
//}
