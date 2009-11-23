#include <iostream>
#include <string>
#include <ctime>

#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <flite/flite.h>
#include <AL/alut.h>

#include "twat.hpp"

// sound
extern "C" cst_voice *register_cmu_us_kal();
size_t NUM_SPEAKERS = 20;
cst_voice *voice;
ALuint *buffer;
ALuint *source;

// helper
void init();
void close();
void play_text(const std::string& s, size_t speaker);

// threads
struct Fetcher
{
    Fetcher(Twat& t) : 
        twat(t), run(true), thread(boost::bind(&Fetcher::func, this)) {}
    ~Fetcher() { run = false; thread.join(); }

    void func()
    {
        boost::xtime xt;
        while (run)
        {
            if (twat.tweetcount() < 3 * NUM_SPEAKERS) 
            {
                try { 
                    twat.get_tweets(NUM_SPEAKERS); 
                } catch (const char* e) { 
                    std::cerr << "Cannot get tweets: " << e << std::endl; 
                }
            }
            boost::xtime_get(&xt, boost::TIME_UTC);
            xt.sec += 1;
            boost::thread::sleep(xt);
        }
    }

    Twat& twat;
    bool run;
    boost::thread thread;
};

struct Speaker
{
    Speaker(Twat& t) :
        twat(t), run(true), thread(boost::bind(&Speaker::func, this))  {}
    ~Speaker() { run = false; thread.join(); }

    void func()
    {
        ALint val;
        boost::xtime xt;
        while (run) 
        {
            for (size_t i = 0; i < NUM_SPEAKERS; ++i) 
            {
                alGetSourcei(source[i], AL_SOURCE_STATE, &val);
                if (val != AL_PLAYING && twat.has_tweets())
                    play_text(twat.next_tweet(), i);
            }
            boost::xtime_get(&xt, boost::TIME_UTC);
            xt.sec += 1;
            boost::thread::sleep(xt);
        }
    }

    Twat& twat;
    bool run;
    boost::thread thread;
};

int main(int argc, char **argv)
{
    bool fake_fetch = false;
    if (argc > 1) {
        for (int i = 0; i < argc; ++i) {
            if (argv[i][0] == '-') {
                fake_fetch |= (std::string(argv[i]) == "-fake");
            } else {
                std::stringstream ss(argv[i]);
                ss >> NUM_SPEAKERS;
            }
        }
    }

    try { init(); } catch (const char* e) 
    {
        std::cerr << "Error: " << e << std::endl;
        close();
        return 1;
    }

    Twat twat(fake_fetch);
    Fetcher fetcher(twat);
    Speaker speaker(twat);

    char tmp;
    std::cout << NUM_SPEAKERS << " Speakers. Type anything to quit... " << std::endl;
    std::cin.get(tmp);

    close();
    return 0;
}

void init()
{
    srand(time(NULL));
    voice = NULL;
    flite_init();
    voice = register_cmu_us_kal();
    if (!voice)
        throw "no voice!";

    alutInit(0, NULL);
    alGetError();

    buffer = new ALuint[NUM_SPEAKERS];
    alGenBuffers(NUM_SPEAKERS, buffer);
    if (alGetError() != AL_NO_ERROR) {
        delete[] buffer;
        throw "no buffer!";
    }

    source = new ALuint[NUM_SPEAKERS];
    alGenSources(NUM_SPEAKERS, source);
    if (alGetError() != AL_NO_ERROR) 
    {
        alDeleteBuffers(NUM_SPEAKERS, buffer);
        delete[] buffer;
        delete[] source;
        throw "no source!";
    }
}

void close()
{
    alSourceStopv(NUM_SPEAKERS, source);
    for (size_t i = 0; i < NUM_SPEAKERS; ++i)
        alSourcei(source[i], AL_BUFFER, NULL);
    alDeleteBuffers(NUM_SPEAKERS, buffer);
    alDeleteSources(NUM_SPEAKERS, source);
    alutExit();

    delete[] buffer;
    delete[] source;
}

void play_text(const std::string& s, size_t speaker)
{
    if (speaker >= NUM_SPEAKERS)
        return;

    alSourcei(source[speaker], AL_BUFFER, NULL);
    cst_wave* w = flite_text_to_wave(s.c_str(), voice);

    ALuint bytes = w->num_samples * w->num_channels * sizeof(short);
    alBufferData(buffer[speaker], AL_FORMAT_MONO16, w->samples, bytes, w->sample_rate);
    delete_wave(w);

    alSourcei(source[speaker], AL_BUFFER, buffer[speaker]);
    alSourcePlay(source[speaker]);
}

