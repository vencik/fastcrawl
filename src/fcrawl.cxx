/**
 *  \file
 *  \brief  FastCrawl CLI
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

#include "libfastcrawl/fastcrawl.hxx"

#include <iostream>
#include <chrono>
#include <exception>
#include <stdexcept>
#include <cstdint>

extern "C" {
#include <getopt.h>
}


// Actual main implementation
static int main_impl(int argc, char * const argv[]) {
    // Startup timestamp
    const auto start_tstmp = std::chrono::system_clock::now();

    // Options & arguments
    bool        verbose = false;
    size_t      tlimit  = SIZE_MAX;
    std::string uri_str = "www.meetangee.com";

    // Usage
    auto usage = [&argv, &uri_str](std::ostream & out) {
        out << "Usage: " << argv[0] << " [OPTIONS] [URI]" << std::endl
            << std::endl
            << "OPTIONS:" << std::endl
            << "    -h or --help                show this help and exit"     << std::endl
            << "    -t or --thread-limit <n>    limit the number of threads" << std::endl
            << "    -v or --verbose             verbose logging to stderr"   << std::endl
            << std::endl
            << "Default URI: " << uri_str << std::endl
            << std::endl
            << "Note that the content is downloaded into the current directory" << std::endl
            << "to files named to indicate the content URI position"            << std::endl
            << "as XXXXXXXX_YYYYYYYY (line and column)."                        << std::endl
            << "As the amount of files may be substantial, consider chaning"    << std::endl
            << "to a new directory before running this."                        << std::endl
            << std::endl;
    };

    // Options handling
    static const struct option long_opts[] {
        { "help",         no_argument,       nullptr, 'h' },
        { "thread-limit", required_argument, nullptr, 't' },
        { "verbose",      no_argument,       nullptr, 'v' },

        { nullptr,        0,                 nullptr, '\0' }  // terminator
    };

    for (;;) {
        int long_opt_ix;
        int opt = ::getopt_long(argc, argv, "htv", long_opts, &long_opt_ix);
        if (-1 == opt) break;  // no more options

        switch (opt) {
            case 'h':   // help
                usage(std::cout);
                return 0;

            case 't':   // thread limit
                tlimit = ::atoi(::optarg);
                break;

            case 'v':   // verbose logging
                verbose = true;
                break;

            default:    // internal error (option handing faulty)
                throw std::logic_error("INTERNAL ERROR: option handling fault");
        }
    }

    // URI argument
    if (::optind < argc) uri_str = argv[::optind++];

    // Unexpected argument
    if (::optind < argc) {
        std::cerr
            << "Unexpected argument: " << argv[::optind] << std::endl
            << std::endl;

        usage(std::cerr);
        return 1;
    }

    // Download (nested scope forcing destructors execution before timestamp)
    {
        // Initialisation
        const auto uri = fastcrawl::uri::parse(uri_str);

        fastcrawl::download     download(uri, "./index.html");
        fastcrawl::html_crawler html_crawler(uri.host, tlimit);

        // Set logging
        download.verbose_log(verbose);
        html_crawler.verbose_log(verbose);

        // Download startup timestamp
        const auto download_start_tstmp = std::chrono::system_clock::now();

        download(html_crawler);  // crawl the index page during download
        html_crawler.wait();     // wait for all refs to be downloaded

        // Download duration
        std::chrono::duration<double> download_time_s =
            std::chrono::system_clock::now() - download_start_tstmp;

        html_crawler.report();   // report the result

        std::cout
            << "Total download time: " << download_time_s.count() << " s"
            << std::endl;
    }

    // Runtime duration
    std::chrono::duration<double> run_time_s =
        std::chrono::system_clock::now() - start_tstmp;

    std::cout
        << "Total run time: " << run_time_s.count() << " s"
        << std::endl;

    return 0;
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
