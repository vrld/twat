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
#include <time.h>

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
    int channel_count;
    cst_wave** w;
    long* pos;
    int* done;
    double* rate_delay;
    double* cur_delay;
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
    int i;

    if (si.w != NULL)
    {
        for (i = 0; i < si.channel_count; ++i)
            delete_wave(si.w[i]);
        free(si.w);
        si.w = NULL;
    }

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
    int chan;
    cst_wave* wave;

    for (i = 0; i < frames; ++i) 
    {
        for (chan = 0; chan < stream->channel_count; ++chan) 
        {
            wave = stream->w[chan];
            if (wave->num_samples - stream->pos[chan] <= 0)
            {
                stream->done[chan] = 1;
                *out++ = (1 << (sizeof(short) - 1));
            } 
            else
            {
                if (!stream->done[chan])
                    *out++ = wave->samples[ stream->pos[chan] ];
                else
                    *out++ = (1 << (sizeof(short) - 1));
            }

            /* hack for certain sound cards that dont support low sample rates... 
             * also: slow down/speed up voices for a more lively experience */
            stream->cur_delay[chan] += 1.;
            if (stream->cur_delay[chan] >= stream->rate_delay[chan])
            {
                stream->pos[chan]++;
                stream->cur_delay[chan] -= stream->rate_delay[chan];
            }
        }
    }

    return 0;
}

void set_output_parameters(PaStreamParameters *params, int device)
{
    const PaDeviceInfo *dev_info;

    params->device = device;
    dev_info = Pa_GetDeviceInfo( params->device );
    si.channel_count = dev_info->maxOutputChannels;

    printf("Using device \"%s\" (%d channels)\n",
            dev_info->name, si.channel_count);

    params->channelCount              = si.channel_count;
    params->sampleFormat              = paInt16;
    params->suggestedLatency          = dev_info->defaultLowOutputLatency;
    params->hostApiSpecificStreamInfo = NULL;
}

void receive_tweet(char* buffer, struct sockaddr_in* server)
{
    struct sockaddr_in client;
    int transfered = 0, timeoutcount = 0;

    /* Get message from message server */
    if (!udp_send(sock, "Gimmeh!", server, sizeof(struct sockaddr_in)))
        error_and_exit("Cannot send message to server", __FILE__, __LINE__);

    do {
        transfered = udp_receive(sock, buffer, BUFSIZE, &client, sizeof(client));
        if (transfered == -1) 
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK) 
            {
                ++timeoutcount;
                if (timeoutcount >= MAX_TIMEOUTS) 
                {
                    printf("Maximum timeouts exceeded. Server is considered dead.");
                    exit(0);
                }
                printf("Server timed out (%d).\n", timeoutcount);
            }

            error_and_exit("Cannot read from server", __FILE__, __LINE__);
        } 
        else if (transfered == 0) 
        {
            error_and_exit("Server has shut down", __FILE__, __LINE__);
        }
    } while (transfered <= 0); 
    if (server->sin_addr.s_addr != client.sin_addr.s_addr)
        error_and_exit("Received message from unexpected server", __FILE__, __LINE__);
}

void next_tweet(struct sockaddr_in* server, int chan)
{
    char buffer[BUFSIZE+1];
    memset(buffer, BUFSIZE, 0);
    receive_tweet(buffer, server);
    printf("%s\n", buffer);

    if (si.w[chan])
        delete_wave(si.w[chan]);

    si.w[chan]          = flite_text_to_wave(buffer, voice);
    si.done[chan]       = 0;
    si.pos[chan]        = 0;
    si.cur_delay[chan]  = 0.;
    si.rate_delay[chan] = 44100. / (double)si.w[chan]->sample_rate;
    /* play some tweets faster, some slower */
    si.rate_delay[chan] *= (.75 + .5 * (double)(rand() % 256) / 255.);
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
           "    %s [server ip] [port] [device-id]\n"
           "EXAMPLE:\n"
           "    %s 127.0.0.1 12345 1   # play on device 1\n\n", 
           prg, prg);
    list_devices();
    return 0;
}

int main(int argc, char** argv)
{
    struct sockaddr_in server;
    int chan;
    PaStream *stream;
    PaStreamParameters output_parameters;
    PaError err;

    voice = NULL;
    srand(time(NULL));
    atexit(at_exit);
    set_signal_handlers();
    init();

    if (argc != 4)
        return usage(argv[0]);

    set_output_parameters(&output_parameters, atoi(argv[3]));

    /* connect to server */
    sock = udp_create_socket(&server, sizeof(server), inet_addr(argv[1]), atoi(argv[2]), WAIT_FOR_RECEIVE);
    if (sock < 0)
        error_and_exit("Cannot create socket", __FILE__, __LINE__);
    
    /* create speech thingies */
    si.w          = malloc(sizeof(cst_wave) * si.channel_count);
    si.pos        = malloc(sizeof(long) * si.channel_count);
    si.done       = malloc(sizeof(int) * si.channel_count);
    si.cur_delay  = malloc(sizeof(double) * si.channel_count);
    si.rate_delay = malloc(sizeof(double) * si.channel_count);
    for (chan = 0; chan < si.channel_count; ++chan) {
        si.w[chan] = NULL;
        next_tweet(&server, chan);
    }

    err = Pa_OpenStream(&stream, NULL, &output_parameters,
            44100., 0, paNoFlag, say_text_callback, &si);
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
    
    while (1)
    {
        usleep(100);
        for (chan = 0; chan < si.channel_count; ++chan) 
        {
            if (si.done[chan])
                next_tweet(&server, chan);
        }
    }
    //Pa_StopStream(stream);

    return 0;
}
