#include "download.hxx"
#include "utility.hxx"

extern "C" {
#include <curl/curl.h>
}

#include <cassert>


namespace fastcrawl {

struct curl_userdata {
    online_data_processor * processor;
    std::FILE             * file;
};  // end of struct curl_userdata


static size_t curl_write(
    void * ptr,
    size_t size,
    size_t nmemb,
    void * userdata)
{
    auto * udata = reinterpret_cast<curl_userdata *>(userdata);
    assert(udata && udata->processor && udata->file);

    (*udata->processor)((unsigned char *)ptr, size * nmemb);
    return std::fwrite(ptr, size, nmemb, udata->file);
}


bool download::run(online_data_processor * processor) const {
    // Initialise curl
    auto * curl = ::curl_easy_init();
    if (nullptr == curl) return false;  // failed to create CURL handle

    // Prepare file stream
    std::FILE * file = std::fopen(m_filename.c_str(), "wb");
    if (nullptr == file) return false;
    run_at_eos([file]() { std::fclose(file); });

    // Run download
    ::curl_easy_setopt(curl, CURLOPT_URL,            m_uri.c_str());
    ::curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);

    if (nullptr != processor) {
        curl_userdata userdata = {processor, file};

        ::curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &curl_write);
        ::curl_easy_setopt(curl, CURLOPT_WRITEDATA,     &userdata);
    }
    else {
        ::curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &std::fwrite);
        ::curl_easy_setopt(curl, CURLOPT_WRITEDATA,     file);
    }

    return CURLE_OK == ::curl_easy_perform(curl);
};

}  // end of namespace fastcrawl
