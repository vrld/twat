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

#include <portaudio.h>
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
int sock = -1;

struct stream_info {
    cst_wave* w;
    long pos;
    int channel;
    int channel_count;
};

static struct stream_info si;

void init()
{
    PaError err;
    if (paNoError != (err = Pa_Initialize())) 
    {
        fprintf(stderr, "Cannot initialize PortAudio: %s\n", Pa_GetErrorText(err));
        exit(-1);
    }

    si.w = NULL;
    si.pos = 0;

    flite_init();
    voice = VOICE();
    CHECK_NULL(voice, "cannot open voice");
}

void at_exit()
{
    Pa_Terminate();

    if (voice) {
        delete_voice(voice);
        voice = NULL;
    }
    
    if (sock >= 0)
        close(sock);
}

static int say_text_callback(const void* in, void* output,
        unsigned long frames, const PaStreamCallbackTimeInfo *timeinfo,
        PaStreamCallbackFlags flags, void* udata)
{
    (void)in; (void)timeinfo; (void)flags;
    struct stream_info* stream = (struct stream_info*)udata;
    short* out = (short*)output;
    unsigned int i;
    int k;

    for (i = 0; i < frames; ++i) 
    {
        if (stream->w->num_samples - stream->pos - i <= 0)
            return paComplete;

        for (k = 0; k < stream->channel_count; ++k) 
        {
            if (k == stream->channel)
                *out++ = stream->w->samples[stream->pos];
            else 
                *out++ = 0x0FFF;
        }
        stream->pos++;
    }

    return 0;
}

void list_devices()
{
    const PaDeviceInfo* info;
    PaDeviceIndex dev, dev_count = Pa_GetDeviceCount();

    printf("Devices:\n");
    for (dev = 0; dev < dev_count; ++dev)
    {
        info = Pa_GetDeviceInfo(dev);
        printf("   [%02d] \"%s\" (%d channels)\n", 
                dev, info->name, info->maxOutputChannels);
    }
}

int usage(char* prg)
{
    printf("USAGE:\n"
           "    %s -devices\n"
           "    %s [server ip] [port] [device-id] [channel]\n"
           "EXAMPLE:\n"
           "    %s -devices             # lists available devices\n"
           "    %s 127.0.0.1 12345 1 2  # play on device 1 on channel 2\n\n", 
           prg, prg, prg, prg);
    list_devices();
    return 0;
}

int main(int argc, char** argv)
{
    struct sockaddr_in server, client;
    char buffer[BUFSIZE+1];
    int transfered = 0, timeoutcount = 0;
    const PaDeviceInfo *dev_info;
    PaStream *stream;
    PaStreamParameters output_parameters;
    PaError err;

    voice = NULL;
    atexit(at_exit);
    set_signal_handlers();
    init();

    if (argc == 2) {
        if (strstr(argv[1], "-devices") == NULL)
            return usage(argv[0]);
        list_devices();
        return 0;
    }

    if (argc != 5)
        return usage(argv[0]);

    /* prepare output stream */
    output_parameters.device = atoi(argv[3]);
    dev_info = Pa_GetDeviceInfo( output_parameters.device );
    printf("Using device \"%s\"\n", dev_info->name);

    si.channel_count = dev_info->maxOutputChannels;
    si.channel = atoi(argv[4]);
    if (si.channel < 0 || si.channel >= si.channel_count) {
        fprintf(stderr, "Invalid channel %d. Choose channel between 0 and %d\n",
                si.channel, si.channel_count);
        exit(-1);
    }

    output_parameters.channelCount = si.channel_count;
    output_parameters.sampleFormat = paInt16;
    output_parameters.suggestedLatency = dev_info->defaultLowOutputLatency;
    output_parameters.hostApiSpecificStreamInfo = NULL;

    /* connect to server */
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

        printf("%s\n", buffer);

        /* play tweet and wait for it to finish */
        if (si.w) {
            delete_wave(si.w);
            si.w = NULL;
            si.pos = 0;
        }

        si.w = flite_text_to_wave(buffer, voice);
        err = Pa_OpenStream(&stream, NULL, &output_parameters,
                (double)si.w->sample_rate, 0, paNoFlag, say_text_callback, &si);
        if (paNoError != err) {
            fprintf(stderr, "Cannot open stream: %s\n", Pa_GetErrorText(err));
            exit(-1);
        }
/*        Pa_OpenDefaultStream(&stream, 0, si.channel_count, paInt16,
                si.w->sample_rate, 0, say_text_callback, &si); */
        err = Pa_StartStream(stream);
        if (paNoError != err) {
            fprintf(stderr, "Cannot start stream: %s\n", Pa_GetErrorText(err));
            exit(-1);
        }
        
        do {
            usleep(100);
        } while (Pa_IsStreamActive(stream));
    }

    return 0;
}
