#ifndef fastcrawl__adler32_hxx
#define fastcrawl__adler32_hxx

#include "online_data_processor.hxx"

#include <cstdint>

extern "C" {
#include <zlib.h>
}


namespace fastcrawl {

class adler32: public online_data_processor {
    private:

    ::uLong    m_checksum;
    uint32_t & m_result;

    public:

    adler32(uint32_t & result);

    void operator () (unsigned char * data, size_t size);

    ~adler32() { m_result = m_checksum; }

};  // end of class adler32

}  // end of namespace fastcrawl

#endif  // end of #ifndef fastcrawl__adler32_hxx
