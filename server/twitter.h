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
#ifndef TWITTER_H
#define TWITTER_H

#include <curl/curl.h>
#include "../error.h"
#include "queue.h"

typedef struct {
    CURL* curl;
    char* error[CURL_ERROR_SIZE];
    char* buffer;
    Queue* messages;
} Twitter_api;

Twitter_api* twitter_new();
void twitter_delete(Twitter_api *T);
void fetch_tweets(Twitter_api* T, size_t tweets_per_trend);
char* get_tweet(Twitter_api* T);

#endif /* TWITTER_H */
