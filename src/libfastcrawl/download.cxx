/**
 *  \file
 *  \brief  URI content downloader
 *
 *  \date   2018/03/27
 *  \author Vaclav Krpec  <vencik@razdva.cz>
 *
 *
 *  LEGAL NOTICE
 *
 *  Copyright (c) 2018, Vaclav Krpec
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *
 *  1. Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the distribution.
 *
 *  3. Neither the name of the copyright holder nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 *  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 *  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER
 *  OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 *  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 *  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 *  OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 *  WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 *  OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 *  EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "download.hxx"
#include "utility.hxx"

extern "C" {
#include <curl/curl.h>
}

#include <iostream>
#include <cassert>


namespace fastcrawl {

/** cURL user data */
struct curl_userdata {
    online_data_processor * processor;  /**< Online data processor */
    std::FILE             * file;       /**< Output file handle    */

};  // end of struct curl_userdata


/**
 *  \brief  cURL write callback
 *
 *  Callback for online data processor injection execution.
 *
 *  \param  ptr       Data chunk member array
 *  \param  size      Data chunk member size
 *  \param  nmemb     Number of members in the array
 *  \param  userdata  Callback data (see \ref curl_userdata)
 *
 *  \return Size of data appended to the output file
 */
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

    // Prepare URI
    std::string uri_str = m_uri;
    ::curl_easy_setopt(curl, CURLOPT_URL, uri_str.c_str());

    // Prepare headers
    struct ::curl_slist * headers = nullptr;
    run_at_eos([headers]() { if (headers) ::curl_slist_free_all(headers); });

    headers = ::curl_slist_append(headers, ("Host: " + m_uri.host).c_str());

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
    VLOG
        << "Downloading URI \"" << uri_str
        << "\", Host: \"" << m_uri.host
        << "\", storing as " << m_filename
        << std::endl;

    const auto curl_res = ::curl_easy_perform(curl);

    if (CURLE_OK != curl_res) {
        LOG
            << "Download FAILED: URI \"" << uri_str
            << "\", Host: \"" << m_uri.host
            << "\" (stored as " << m_filename
            << "): " << curl_res
            << ": " << curl_easy_strerror(curl_res)
            << std::endl;

        return false;
    }

    return true;  // all OK :-)
};

}  // end of namespace fastcrawl
