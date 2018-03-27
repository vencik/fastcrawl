#ifndef fastcrawl__adler32_hxx
#define fastcrawl__adler32_hxx

/**
 *  \file
 *  \brief  Adler32 online checksum
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

#include <cstdint>

extern "C" {
#include <zlib.h>
}


namespace fastcrawl {

/**
 *  \brief  Online data checksum based on Adler32 algorithm
 *
 *  The data processor uses Zlib implementation of Adler32 fast checksum
 *  function.
 *
 *  See https://en.wikipedia.org/wiki/Adler-32
 *  and http://zlib.net/manual.html#Checksum
 */
class adler32: public online_data_processor {
    private:

    ::uLong    m_checksum;  /**< Online checksum */
    uint32_t & m_result;    /**< Final result    */

    public:

    /** Constructor, takes reference to the result */
    adler32(uint32_t & result);

    void operator () (unsigned char * data, size_t size);

    /** Destructor assigns te result */
    ~adler32() { m_result = m_checksum; }

};  // end of class adler32

}  // end of namespace fastcrawl

#endif  // end of #ifndef fastcrawl__adler32_hxx
