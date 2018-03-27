#ifndef fastcrawl__uri_hxx
#define fastcrawl__uri_hxx

/**
 *  \file
 *  \brief  Simple URI parser
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

#include <string>
#include <regex>
#include <cstdint>


namespace fastcrawl {

/**
 *  \brief  Simple regular expression based URI parser
 *
 *  Breaks URI down to its basic parts.
 */
class uri {
    private:

    static const std::regex s_regex;  /**< URI regex */

    public:

    std::string scheme;     /**< Scheme         */
    std::string user;       /**< User name      */
    std::string password;   /**< User password  */
    std::string host;       /**< Authority host */
    uint16_t    port;       /**< Authority port */
    std::string path;       /**< URI path       */
    std::string query;      /**< Query string   */
    std::string fragment;   /**< Fragment       */

    /** Constructor (empty URI) */
    uri():
        port(0)
    {}

    /**
     *  \brief  Constructor
     *
     *  Sets URI by its basic parts.
     */
    uri(const std::string & scheme_,
        const std::string & user_,
        const std::string & password_,
        const std::string & host_,
        uint16_t            port_,
        const std::string & path_,
        const std::string & query_,
        const std::string & fragment_)
    :
        scheme(scheme_),
        user(user_),
        password(password_),
        host(host_),
        port(port_),
        path(path_),
        query(query_),
        fragment(fragment_)
    {}

    /** Constructs URI from string */
    static uri parse(const std::string & uri_);

    /** URI equality comparison */
    bool operator == (const uri & arg) const;

    /** URI non-equality comparison */
    bool operator != (const uri & arg) const { return !((*this) == arg); }

    /** URI serialisation */
    operator std::string () const;

};  // end of class uri

}  // end of namespace fastcrawl

#endif  // and of #ifndef fastcrawl__uri_hxx
