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

#include "uri.hxx"

#include <iostream>
#include <sstream>
#include <cassert>


namespace fastcrawl {

// URI regex
const std::regex uri::s_regex(
    "^"
    "(([A-Za-z%0-9]+)://)?"                 // 2:  scheme
    "(([A-Za-z%0-9]+)(:([A-Za-z0-9]+))@)?"  // 4:  user, 6: password
    "([A-Za-z%0-9.-]+)?"                    // 7:  host
    "(:([0-9]+))?"                          // 9:  port
    "([^?#]*)"                              // 10: path
    "(\\?([^#]*))?"                         // 12: query
    "(#(.*))?"                              // 14: fragment
    "$"
);


uri uri::parse(const std::string & uri_) {
    std::smatch bref;

    // Invalid URI
    if (!std::regex_match(uri_, bref, s_regex))
        return uri(
            std::string(),
            std::string(),
            std::string(),
            std::string(),
            0,
            std::string(),
            std::string(),
            std::string());

    // Port (defaults)
    uint16_t port = 0;
    if (!bref[9].str().empty()) {
        std::stringstream port_ss(bref[9]);
        port_ss >> port;
        assert(!port_ss.fail());
    }

    return uri(
        bref[2],
        bref[4],
        bref[6],
        bref[7],
        port,
        bref[10],
        bref[12],
        bref[14]);
}


bool uri::operator == (const uri & arg) const {
    return
        scheme   == arg.scheme      &&
        user     == arg.user        &&
        password == arg.password    &&
        host     == arg.host        &&
        port     == arg.port        &&
        path     == arg.path        &&
        query    == arg.query       &&
        fragment == arg.fragment;
}


uri::operator std::string () const {
    std::stringstream uri_ss;

    if (!scheme.empty()) uri_ss << scheme << "://";

    if (!host.empty()) {
        if (!user.empty()) {
            uri_ss << user;

            if (!password.empty()) uri_ss << ':' << password;

            uri_ss << '@';
        }

        uri_ss << host;

        if (port) uri_ss << ':' << port;
    }

    if (!path.empty())     uri_ss << path;
    if (!query.empty())    uri_ss << '?' << query;
    if (!fragment.empty()) uri_ss << '#' << fragment;

    return uri_ss.str();
}

}  // end of namespace fastcrawl
