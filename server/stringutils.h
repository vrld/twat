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
#ifndef STRINGUTILS_H
#define STRINGUTILS_H

#define char_replace(str, s, r)  do { char *p; \
    for (p = str; *p; ++p) { \
        if ((s) == *p) *p = (r); \
    } } while(0)
char* str_replace(const char* restrict src, const char* restrict s, const char* restrict r);
char* str_replace_all(const char* restrict src, const char** restrict sr);
//char* str_replace_char(const char* restrict src, char s, const char* restrict r);

#endif /* STRINGUTILS_H */
