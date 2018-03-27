#include "libfastcrawl/adler32.hxx"

#include <iostream>
#include <iomanip>
#include <cstdint>


// Actual main implementation
static int main_impl(int argc, char * const argv[]) {
    static const std::string Wikipedia("Wikipedia");

    uint32_t checksum;
    {
        fastcrawl::adler32 adler32(checksum);

        auto * data = Wikipedia.data();

        adler32((unsigned char *)data + 0, 3);
        adler32((unsigned char *)data + 3, 2);
        adler32((unsigned char *)data + 5, 4);
    }

    if (0x11e60398 == checksum) return 0;

    std::cerr
        << "Checksum of \"" << Wikipedia << "\" FAILED" << std::endl
        << "\texpected: "
        << std::hex << std::setw(8) << std::setfill('0') << 0x11e60398
        << "\tgot     : "
        << std::hex << std::setw(8) << std::setfill('0') << checksum
        << std::endl;

    return 1;
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
