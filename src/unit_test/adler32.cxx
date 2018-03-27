/**
 *  \file
 *  \brief  Adler32 checksum unit test
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
