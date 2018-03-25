#include "libfastcrawl/fastcrawl.hxx"

#include <iostream>
#include <chrono>
#include <exception>


// Actual main implementation
static int main_impl(int argc, char * const argv[]) {
    const auto start_tstmp = std::chrono::system_clock::now();

    {
        fastcrawl::download download("www.meetangee.com", "./index.html");
        fastcrawl::html_crawler html_crawler;

        const auto download_start_tstmp = std::chrono::system_clock::now();

        download(html_crawler);

        std::chrono::duration<double> download_time_s =
            std::chrono::system_clock::now() - download_start_tstmp;

        std::cout
            << "Total download time: " << download_time_s.count() << " s"
            << std::endl;
    }

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
