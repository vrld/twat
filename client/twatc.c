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

#include <AL/alut.h>
#include <flite/flite.h>
#ifndef VOICE
#define VOICE register_cmu_us_kal
#endif

#include "../error.h"
#include "../signal_handler.h"
#include "../udp.h"

#include <stdio.h>
#include <unistd.h>
#define BUFSIZE 255
#define WAIT_FOR_RECEIVE 5
#define MAX_TIMEOUTS 25

int usleep(unsigned int usec);

cst_voice *VOICE();
cst_voice *voice;
ALuint buffer;
ALuint source;
int sock = -1;

void init()
{
    flite_init();
    voice = VOICE();
    CHECK_NULL(voice, "cannot open voice");

    alutInit(0, NULL);
    alGetError();

    alGenBuffers(1, &buffer);
    ON_AL_ERROR("cannot create audio buffer");

    alGenSources(1, &source);
    ON_AL_ERROR("cannot create audio source");
}

void at_exit()
{
    printf("Shutting down...\n");
    alSourceStop(source);
    alSourcei(source, AL_BUFFER, 0);
    alDeleteSources(1, &source);
    alDeleteBuffers(1, &buffer);
    alutExit();

    if (voice) {
        delete_voice(voice);
        voice = NULL;
    }
    
    if (sock >= 0)
        close(sock);
}

void say_text(const char* text)
{
    cst_wave* w;
    ALuint bytes;

    alSourcei(source, AL_BUFFER, 0);
    ON_AL_ERROR("cannot reset source buffer");

    w = flite_text_to_wave(text, voice);

    bytes = w->num_samples * w->num_channels * sizeof(short);
    alBufferData(buffer, AL_FORMAT_MONO16, w->samples, bytes, w->sample_rate);
    ON_AL_ERROR("cannot set buffer data");
    delete_wave(w);

    alSourcei(source, AL_BUFFER, buffer);
    ON_AL_ERROR("cannot attach buffer to source");
    alSourcePlay(source);
    ON_AL_ERROR("cannot play source")
}

int main(int argc, char** argv)
{
    struct sockaddr_in server, client;
    char buffer[BUFSIZE+1];
    int transfered = 0, timeoutcount = 0;
    ALint source_state;

    if (argc < 3) {
        printf("USAGE: %s [server ip] [port]\n", argv[0]);
        return 0;
    }

    voice = NULL;
    atexit(at_exit);
    set_signal_handlers();
    init();

    sock = udp_create_socket(&server, sizeof(server), inet_addr(argv[1]), atoi(argv[2]), WAIT_FOR_RECEIVE);
    if (sock < 0)
        error_and_exit("Cannot create socket", __FILE__, __LINE__);
    
    for (;;) {
        /* Get message from message server */
        if (!udp_send(sock, "Gimmeh!", &server, sizeof(server)))
            error_and_exit("Cannot send message to server", __FILE__, __LINE__);

        transfered = udp_receive(sock, buffer, BUFSIZE, &client, sizeof(client));
        if (transfered == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                ++timeoutcount;
                if (timeoutcount >= MAX_TIMEOUTS) {
                    printf("Maximum timeouts exceeded. Server is considered dead.");
                    exit(0);
                }
                printf("Server timed out (%d).\n", timeoutcount);
                continue;
            } else {
                error_and_exit("Cannot read from server", __FILE__, __LINE__);
            }
        } else if (transfered == 0) {
                error_and_exit("Server has shut down", __FILE__, __LINE__);
        }
        if (server.sin_addr.s_addr != client.sin_addr.s_addr)
            error_and_exit("Received message from unexpected server", __FILE__, __LINE__);

        timeoutcount = 0;

        //printf("%s\n", buffer);

        /* play tweet and wait for it to finish */
        say_text(buffer);
        do {
            usleep(100);
            alGetSourcei(source, AL_SOURCE_STATE, &source_state);
        } while (source_state == AL_PLAYING);
    }

    return 0;
}
