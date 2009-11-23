#ifndef TWAT_HPP
#define TWAT_HPP

#include <curl/curl.h>
#include <string>
#include <map>
#include <vector>
#include <boost/thread.hpp>

class Twat
{
    public:
        typedef std::map<std::string, std::string> Trends;
        typedef std::vector<std::string> Tweets;

        Twat();
        ~Twat();
        void get_tweets(size_t per_trend);
        std::string next_tweet();
        inline bool has_tweets() const { return !tweets_.empty(); }
        inline size_t tweetcount() const { return tweets_.size(); }

    private:
        static int writer(char *data, size_t size, size_t nmemb, Twat* self);
        const std::string& fetch_url(const std::string& url);

        boost::mutex mutex_;

        CURL*       curl_;
        char*       error_buffer_[CURL_ERROR_SIZE];
        std::string buffer_;
        Trends      trends_;
        Tweets      tweets_;

        bool do_fake_fetch_;
};

#endif //TWAT_HPP
