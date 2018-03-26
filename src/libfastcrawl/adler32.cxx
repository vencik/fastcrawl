#include "adler32.hxx"


namespace fastcrawl {

adler32::adler32(uint32_t & result):
    m_checksum(0),
    m_result(result)
{
    //uLong m_checksum = ::adler32(0L, Z_NULL, 0);
}


void adler32::operator () (unsigned char * data, size_t size) {
    //m_checksum = ::adler32(m_checksum, data, size);
    m_checksum += size;
}

}  // end of namespace fastcrawl
