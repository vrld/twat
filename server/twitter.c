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
#include "twitter.h"
#include "cJSON.h"
#include "stringutils.h"
#include <string.h>
#include <stdio.h>

extern int snprintf(char*, size_t, const char*, ...);

static size_t curl_write_(char *data, size_t size, size_t nmemb, Twitter_api* T)
{
    size_t buflen = 0;
    CHECK_NULL(data, "no data to write?");
    CHECK_NULL(T, "the whale failed while writing");

    if (T->buffer)
        buflen = strlen(T->buffer) * sizeof(char);

    T->buffer = (char*)realloc(T->buffer, buflen + size * nmemb + 1);
    CHECK_NULL(T->buffer, "cannot increase buffer size!");

    memcpy(T->buffer + buflen, data, size*nmemb);
    T->buffer[buflen + size*nmemb] = '\0';

    return size * nmemb;
}

static void fetch_url_(Twitter_api *T, const char* url)
{
    CURLcode result;
    CHECK_NULL(T, "whale fail");
    if (T->buffer) {
        free(T->buffer);
        T->buffer = NULL;
    }

    curl_easy_setopt(T->curl, CURLOPT_URL, url);
    result = curl_easy_perform(T->curl);

    if (result != CURLE_OK)
        fprintf(stderr, "cannot fetch url '%s': %s\n", url, *(T->error));
}

Twitter_api* twitter_new()
{
    Twitter_api* T;
    T = (Twitter_api*)malloc(sizeof(Twitter_api));
    CHECK_NULL(T, "the whale has failed");

    T->curl = curl_easy_init();
    CHECK_NULL(T->curl, "no curling");
    curl_easy_setopt(T->curl, CURLOPT_ERRORBUFFER,    T->error);
    curl_easy_setopt(T->curl, CURLOPT_WRITEFUNCTION,  curl_write_);
    curl_easy_setopt(T->curl, CURLOPT_WRITEDATA,      T);
    curl_easy_setopt(T->curl, CURLOPT_HEADER,         0);
    curl_easy_setopt(T->curl, CURLOPT_VERBOSE,        0);
    curl_easy_setopt(T->curl, CURLOPT_NOPROGRESS,     1);
    curl_easy_setopt(T->curl, CURLOPT_FOLLOWLOCATION, 1);
    
    T->buffer = NULL;

    T->messages = queue_new();
    return T;
}

void twitter_delete(Twitter_api *T)
{
    CHECK_NULL(T, "twitter api was NULL");
    queue_delete(T->messages);
    curl_easy_cleanup(T->curl);
}

int get_search_url_(char **url, size_t tweets_per_trend, const char *tweet_url)
{
    static const char* tweets_url_fmt = "http://search.twitter.com/search.json%s&lang=en&rpp=%d";
    const int num_len = tweets_per_trend / 10 + 1;
    char* query_str;
    query_str = strstr(tweet_url, "?q=");
    *url = (char*)malloc(strlen(query_str) + strlen(tweets_url_fmt) - 3 + num_len);
    return sprintf(*url, tweets_url_fmt, query_str, tweets_per_trend); 
}

void attach_message_(Twitter_api *T, const cJSON* msg)
{
    const char* replacements[] = { 
        "&amp;",   "&",
        "&quot;",  "\"",
        "&lt;",    "<",
        "&gt;",    ">",
        "http://", "",
        NULL,      NULL};

    char* cleaned = str_replace_all(msg->valuestring, replacements);
    char_replace(cleaned, '#', ' ');

    enqueue(T->messages, cleaned);
    free(cleaned);
}

void fetch_tweets(Twitter_api* T, size_t tweets_per_trend)
{
    static const char* trends_url = "http://search.twitter.com/trends.json";
    cJSON *trends_rt, *trends, *tweets_rt, *tweets;
    char *tweet_search_url = NULL;
    int i, k;

    fetch_url_(T, trends_url);
    trends_rt = cJSON_Parse(T->buffer);
    trends = cJSON_GetObjectItem(trends_rt, "trends");
    for (i = 0; i < cJSON_GetArraySize(trends); ++i) {
        get_search_url_(&tweet_search_url, tweets_per_trend,
                cJSON_GetObjectItem(cJSON_GetArrayItem(trends, i), "url")->valuestring);
        fetch_url_(T, tweet_search_url);

        tweets_rt = cJSON_Parse(T->buffer);
        tweets = cJSON_GetObjectItem(tweets_rt, "results");

        for (k = 0; k < cJSON_GetArraySize(tweets); ++k) {
            attach_message_(T, cJSON_GetObjectItem(cJSON_GetArrayItem(tweets, k), "text"));
        }

        cJSON_Delete(tweets_rt);
        free (tweet_search_url);
        tweet_search_url = NULL;
    }

    cJSON_Delete(trends_rt);
}

char* get_tweet(Twitter_api* T)
{
    char* ret;
    const char* temp;
    if (queue_empty(T->messages))
        fetch_tweets(T, 5);

    temp = front(T->messages);
    ret = malloc(strlen(temp)+1);
    strcpy(ret, temp);
    dequeue(T->messages);

    return ret;
}
