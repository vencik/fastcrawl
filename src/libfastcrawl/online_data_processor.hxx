#ifndef fastcrawl__online_data_processor_hxx
#define fastcrawl__online_data_processor_hxx

#include <cstddef>


namespace fastcrawl {

class online_data_processor {
    public:

    virtual void operator () (unsigned char * data, size_t size) = 0;

    virtual ~online_data_processor() {}

};  // end of class online_data_processor

}  // end of namespace fastcrawl

#endif  // end of #ifndef fastcrawl__online_data_processor_hxx
