#ifndef fastcrawl__online_data_processor_hxx
#define fastcrawl__online_data_processor_hxx

#include <utility>
#include <cstddef>


namespace fastcrawl {

class online_data_processor {
    public:

    virtual void operator () (unsigned char * data, size_t size) = 0;

    virtual ~online_data_processor() {}

};  // end of class online_data_processor


template <class Proc, class... Procs>
class compound_data_processor: public online_data_processor {
    private:

    using subproc_t = compound_data_processor<Procs...>;

    Proc      m_proc;
    subproc_t m_procs;

    public:

    compound_data_processor(Proc && proc, Procs &&... procs):
        m_proc(std::forward<Proc>(proc)),
        m_procs(std::forward<Procs>(procs)...)
    {}

    void operator () (unsigned char * data, size_t size) {
        m_proc(data, size);
        m_procs(data, size);
    }

};  // end of template class compound_data_processor

template <class Proc>
class compound_data_processor<Proc>: public online_data_processor {
    private:

    Proc m_proc;

    public:

    compound_data_processor(Proc && proc):
        m_proc(std::forward<Proc>(proc))
    {}

    void operator () (unsigned char * data, size_t size) {
        m_proc(data, size);
    }

};  // end of template class compound_data_processor specialisation


template <class Proc, class... Procs>
compound_data_processor<Proc, Procs...> data_processor(
    Proc  &&     proc,
    Procs &&... procs)
{
    return compound_data_processor<Proc, Procs...>(
        std::forward<Proc>(proc),
        std::forward<Procs>(procs)...);
}

}  // end of namespace fastcrawl

#endif  // end of #ifndef fastcrawl__online_data_processor_hxx
