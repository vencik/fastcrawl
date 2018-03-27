/**
 *  \file
 *  \brief  URI parser unit test
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

#include "libfastcrawl/uri.hxx"

#include <iostream>
#include <list>


/** URI parser unit test */
class uri_parser_test {
    private:

    /** Test case */
    struct test_case {
        const std::string    uri_str;   /**< URI string              */
        const fastcrawl::uri uri;       /**< Expected parsing result */

        test_case(
            const std::string uri_str_,
            fastcrawl::uri && uri_)
        :
            uri_str(uri_str_),
            uri(uri_)
        {}

    };  // end of struct test_case

    std::list<test_case> m_test_cases;  /**< Test cases */

    public:

    /** Create URI parser unit test cases */
    uri_parser_test() {
        m_test_cases.emplace_back(
            "www.meetangee.com",
            fastcrawl::uri("", "", "", "www.meetangee.com", 0, "", "", "")
        );
        m_test_cases.emplace_back(
            "https://github.com/vencik",
            fastcrawl::uri("https", "", "", "github.com", 0, "/vencik", "", "")
        );
        m_test_cases.emplace_back(
            "http://bob:secret@webproxy.example.com:8080/",
            fastcrawl::uri("http", "bob", "secret", "webproxy.example.com", 8080, "/", "", "")
        );
        m_test_cases.emplace_back(
            "https://www.example.com:8443/my/path/some.js?abc=123#whatever",
            fastcrawl::uri("https", "", "", "www.example.com", 8443, "/my/path/some.js", "abc=123", "whatever")
        );
        m_test_cases.emplace_back(
            "#whatever",
            fastcrawl::uri("", "", "", "", 0, "", "", "whatever")
        );
    }

    /** Execute URI parser unit test */
    bool operator () () const {
        size_t test_cnt = 0;
        size_t fail_cnt = 0;

        for (auto & test_case: m_test_cases) {
            ++test_cnt;

            const auto uri = fastcrawl::uri::parse(test_case.uri_str);
            if (uri != test_case.uri) {
                std::cerr
                    << "URI parsing failed for \"" << test_case.uri_str << "\""
                    << std::endl
                    << "\texpected \"" << (std::string)test_case.uri << "\""
                    << std::endl
                    << "\tgot      \"" << (std::string)uri << "\""
                    << std::endl;

                ++fail_cnt;
            }
        }

        std::cerr
            << "URI parsing UT: "
            << fail_cnt << "/" << test_cnt << " failed"
            << std::endl;

        return 0 == fail_cnt;
    }

};  // end of class uri_parser_test

static const uri_parser_test uri_parser_ut;


// Actual main implementation
static int main_impl(int argc, char * const argv[]) {
    return uri_parser_ut() ? 0 : 1;
}


// Exception safeness wrapper
int main(int argc, char * const argv[]) {
    try {
        return main_impl(argc, argv);
    }

    // Unhandled standard exception
    catch (const std::exception & ex) {
        std::cerr
            << "Stndard exception caught: " << ex.what()
            << std::endl;
    }

    // Unhandled unknown exception
    catch (...) {
        std::cerr
            << "Unknown exception caught"
            << std::endl;
    }

    return 64;  // exception caught
}
