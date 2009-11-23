#include "twat.hpp"
#include <json/json.h>
#include <algorithm>
#include <sstream>

Twat::Twat(bool fake) 
    : do_fake_fetch_(fake)
{
    curl_ = curl_easy_init();
    if (!curl_)
        throw "no curling!";

    curl_easy_setopt(curl_, CURLOPT_ERRORBUFFER,    error_buffer_);
    curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION,  Twat::writer);
    curl_easy_setopt(curl_, CURLOPT_WRITEDATA,      this);
    curl_easy_setopt(curl_, CURLOPT_HEADER,         0);
    curl_easy_setopt(curl_, CURLOPT_VERBOSE,        0);
    curl_easy_setopt(curl_, CURLOPT_NOPROGRESS,     1);
    curl_easy_setopt(curl_, CURLOPT_FOLLOWLOCATION, 1);
}

Twat::~Twat()
{
    if (curl_)
        curl_easy_cleanup(curl_);
}

#include "fake_fetch.h"

void Twat::get_tweets(size_t per_trend)
{
    if (do_fake_fetch_) {
        fake_fetch(per_trend);
        return;
    }

    Json::Value root_tr, root_se;
    Json::Reader reader;

    fetch_url("http://search.twitter.com/trends.json");
    if (reader.parse(buffer_, root_tr)) 
    {
        for (size_t i = 0; i < root_tr["trends"].size(); ++i) 
        {
            std::string url = root_tr["trends"][i]["url"].asString();
            url = "http://search.twitter.com/search.json" + url.substr(url.rfind("?q="));
            trends_[ root_tr["trends"][i]["name"].asString() ] = url;

            std::stringstream ss;
            ss << url << "&lang=en&rpp=" << per_trend;
            const std::string& tmp = fetch_url(ss.str());
            if (reader.parse(tmp, root_se)) 
            {
                for (size_t i = 0; i < root_se["results"].size(); ++i) 
                {
                    boost::mutex::scoped_lock l(mutex_);
                    std::string message = root_se["results"][i]["text"].asString();

                    size_t p;
                    while (std::string::npos != (p = message.find('#'))) 
                        message = message.replace(p, 1, 1, ' ');

                    while (std::string::npos != (p = message.find("&amp;"))) 
                        message = message.replace(p, 5, 1, '&');

                    while (std::string::npos != (p = message.find("&quot;"))) 
                        message = message.replace(p, 6, 1, '"');

                    tweets_.push_back( message );
                }
            }
        }
    }

    std::random_shuffle(tweets_.begin(), tweets_.end());
}

int Twat::writer(char *data, size_t size, size_t nmemb, Twat* self)
{
    if (self == NULL)
        return 0;

    self->buffer_.append(data, size * nmemb);
    return size * nmemb;
}

const std::string& Twat::fetch_url(const std::string& url)
{
    buffer_.clear();
    curl_easy_setopt(curl_, CURLOPT_URL, url.c_str());
    CURLcode result = curl_easy_perform(curl_);

    if (result != CURLE_OK) 
    {
        std::stringstream ss;
        ss << "Curling went wrong (fetch from url: " << url << "):" << std::endl << error_buffer_;
        throw ss.str().c_str();
    }

    return buffer_;
}


std::string Twat::next_tweet() 
{ 
    boost::mutex::scoped_lock l(mutex_);
    std::string t = tweets_.back(); 
    tweets_.pop_back(); 
    return t;
}
