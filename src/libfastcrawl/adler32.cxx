#include "adler32.hxx"


namespace fastcrawl {

adler32::adler32(uint32_t & result):
    m_result(result)
{
    m_checksum = ::adler32(0L, Z_NULL, 0);
}


void adler32::operator () (unsigned char * data, size_t size) {
    m_checksum = ::adler32(m_checksum, data, size);
}

}  // end of namespace fastcrawl
