#include "libfastcrawl/fastcrawl.hxx"

#include <iostream>
#include <chrono>
#include <exception>
#include <stdexcept>

extern "C" {
#include <getopt.h>
}


// Actual main implementation
static int main_impl(int argc, char * const argv[]) {
    // Startup timestamp
    const auto start_tstmp = std::chrono::system_clock::now();

    // Options & arguments
    bool        verbose = false;
    std::string uri_str = "www.meetangee.com";

    // Usage
    auto usage = [&argv, &uri_str](std::ostream & out) {
        out << "Usage: " << argv[0] << " [OPTIONS] [URI]" << std::endl
            << std::endl
            << "OPTIONS:" << std::endl
            << "    -h, --help           show help and exit"        << std::endl
            << "    -v, --verbose        verbose logging to stderr" << std::endl
            << std::endl
            << "Default URI: " << uri_str << std::endl
            << std::endl;
    };

    // Options handling
    static const struct option long_opts[] {
        { "help",       no_argument,    nullptr, 'h' },
        { "verbose",    no_argument,    nullptr, 'v' },

        { nullptr,      0,              nullptr, '\0' }  // terminator
    };

    for (;;) {
        int long_opt_ix;
        int opt = getopt_long(argc, argv, "hv", long_opts, &long_opt_ix);
        if (-1 == opt) break;  // no more options

        switch (opt) {
            case 'h':   // help
                usage(std::cout);
                return 0;

            case 'v':   // verbose logging
                verbose = true;
                break;

            default:    // internal error (option handing faulty)
                throw std::logic_error("INTERNAL ERROR: option handling fault");
        }
    }

    // URI argument
    if (optind < argc) uri_str = argv[optind++];

    // Unexpected argument
    if (optind < argc) {
        std::cerr
            << "Unexpected argument: " << argv[optind] << std::endl
            << std::endl;

        usage(std::cerr);
        return 1;
    }

    // Download (nested scope forcing destructors execution before timestamp)
    {
        // Initialisation
        const auto uri = fastcrawl::uri::parse(uri_str);

        fastcrawl::download     download(uri, "./index.html");
        fastcrawl::html_crawler html_crawler(uri.host);

        if (verbose) {
            download.verbose_log();
            html_crawler.verbose_log();
        }

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
