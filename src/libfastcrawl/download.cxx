#include "download.hxx"
#include "utility.hxx"

extern "C" {
#include <curl/curl.h>
}

#include <iostream>
#include <cassert>


namespace fastcrawl {

struct curl_userdata {
    online_data_processor * processor;
    std::FILE             * file;
};  // end of struct curl_userdata


static bool schemeless(const std::string & uri) {
    const auto scheme_size = uri.find("://");
    return std::string::npos == scheme_size;
}


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
    run_at_eos([curl]() { ::curl_easy_cleanup(curl); });

    // Prepare file stream
    std::FILE * file = std::fopen(m_filename.c_str(), "wb");
    if (nullptr == file) return false;
    run_at_eos([file]() { std::fclose(file); });

    std::cerr
        << "Downloading URI \"" << m_uri
        << "\", Host: " << m_host
        << std::endl;

    const bool no_scheme = schemeless(m_uri);

    // Prepare URI
    if (no_scheme) {
        ::curl_easy_setopt(curl, CURLOPT_URL, (m_host + m_uri).c_str());
    }
    else
        ::curl_easy_setopt(curl, CURLOPT_URL, m_uri.c_str());

    // Prepare headers
    struct ::curl_slist * headers = nullptr;
    run_at_eos([headers]() { if (headers) ::curl_slist_free_all(headers); });

    if (no_scheme && !m_host.empty())
        headers = ::curl_slist_append(headers, ("Host: " + m_host).c_str());

    if (nullptr != headers)
        ::curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    // Other cURL options
    ::curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);  // follow redirects

    // Set response data callback
    if (nullptr != processor) {
        curl_userdata userdata = {processor, file};

        ::curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &curl_write);
        ::curl_easy_setopt(curl, CURLOPT_WRITEDATA,     &userdata);
    }
    else {
        ::curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &std::fwrite);
        ::curl_easy_setopt(curl, CURLOPT_WRITEDATA,     file);
    }

    // Run download
    const auto curl_res = ::curl_easy_perform(curl);

    std::cerr
        << "Download result: " << curl_res
        << ": " << curl_easy_strerror(curl_res)
        << std::endl;

    return CURLE_OK == curl_res;
};

}  // end of namespace fastcrawl
