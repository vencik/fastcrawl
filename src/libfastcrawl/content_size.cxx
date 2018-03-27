#include "content_size.hxx"


namespace fastcrawl {

void content_size::operator () (unsigned char * data, size_t size) {
    m_size += size;
}

}  // end of namespace fastcrawl
