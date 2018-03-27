#ifndef fastcrawl__download_hxx
#define fastcrawl__download_hxx

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

#include "online_data_processor.hxx"
#include "uri.hxx"
#include "logger.hxx"

#include <string>
#include <list>
#include <cstddef>


namespace fastcrawl {

/**
 *  \brief  URI content download
 *
 *  Uses cURL to fetch the content.
 *  It may execute \ref fastcrawl::online_data_processor injection
 *  on each data chunk received.
 *
 *  See https://curl.haxx.se/
 */
class download: public logger {
    private:

    const uri         m_uri;        /**< URI                          */
    const std::string m_filename;   /**< Name of content storage file */

    public:

    /**
     *  \brief  Constructor
     *
     *  \param  uri       Content URI
     *  \param  filename  Content storage file name
     */
    download(
        const uri &         uri_,
        const std::string & filename)
    :
        m_uri(uri_),
        m_filename(filename)
    {}

    /**
     *  \brief  Download execution
     *
     *  \return \c true iff the content was downloaded
     */
    bool operator () () const { return run(nullptr); }

    /**
     *  \brief  Download execution
     *
     *  \param  processor  Online data processor injection
     *
     *  \return \c true iff the content was downloaded
     */
    bool operator () (online_data_processor & processor) const {
        return run(&processor);
    }

    private:

    /**
     *  \brief  Download execution implementation
     *
     *  Instantiates cURL session, assembles required HTTP request fields
     *  and requests the content.
     *  The download is executed in the current thread (blocking reads).
     *
     *  \param  processor  Online data processor injection
     *
     *  \return \c true iff the content was downloaded
     */
    bool run(online_data_processor * processor) const;

};  // end of class download

}  // end of namespace libfastcrawl

#endif  // end of #ifndef fastcrawl__download_hxx
