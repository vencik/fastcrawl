#ifndef fastcrawl__content_size_hxx
#define fastcrawl__content_size_hxx

#include "online_data_processor.hxx"


namespace fastcrawl {

class content_size: public online_data_processor {
    private:

    size_t   m_size;
    size_t & m_result;

    public:

    content_size(size_t & result): m_size(0), m_result(result) {}

    void operator () (unsigned char * data, size_t size);

    ~content_size() { m_result = m_size; }

};  // end of class content_size

}  // end of namespace fastcrawl

#endif  // end of #ifndef fastcrawl__content_size_hxx
