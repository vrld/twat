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
#include <stdlib.h>
#include "../error.h"
#include "../signal_handler.h"
#include "../udp.h"
#include "twitter.h"

#include <stdio.h>
#include <unistd.h>
#define BUFSIZE 255

Twitter_api* T = NULL;
int sock = -1;

void at_exit()
{
    printf("Shutting down...\n");

    if (sock >= 0)
        close(sock);

    twitter_delete(T);
}

int main(int argc, char** argv)
{
    struct sockaddr_in server;
    struct sockaddr_in client;
    char buffer[BUFSIZE];
    socklen_t len_client;
    int transfered = 0;
    char* msg;

    if (argc < 2) {
        printf("USAGE: %s [port]\n", argv[0]);
        return 0;
    }

    atexit(at_exit);
    set_signal_handlers();

    T = twitter_new();
    printf("Reading tweets...\n");
    fetch_tweets(T, 10);

    printf("Opening server...\n");
    sock = udp_create_socket(&server, sizeof(server), htonl(INADDR_ANY), atoi(argv[1]), 0);
    if (sock < 0)
        error_and_exit("cannot create socket", __FILE__, __LINE__);

    if (bind(sock, (struct sockaddr*)&server, sizeof(server)) < 0)
        error_and_exit("Cannot bind to socket", __FILE__, __LINE__);

    len_client = sizeof(client);
    for (;;) {
        transfered = udp_receive(sock, buffer, BUFSIZE, &client, len_client);
        switch (transfered) {
            case -1: 
                error_and_exit("Failed to receive message", __FILE__, __LINE__);
            case 0:
                error_and_exit("Client has shut down...", __FILE__, __LINE__);
        }

        msg = get_tweet(T);
        transfered = udp_send(sock, msg, &client, sizeof(client));
        free(msg);

        if (!transfered)
            error_and_exit("Sending has failed!", __FILE__, __LINE__);
    }

    close(sock);
    return 0;
}
