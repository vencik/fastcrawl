#ifndef fastcrawl__download_hxx
#define fastcrawl__download_hxx

#include "online_data_processor.hxx"
#include "uri.hxx"
#include "logger.hxx"

#include <string>
#include <list>
#include <cstddef>


namespace fastcrawl {

class download: public logger {
    private:

    const uri         m_uri;
    const std::string m_filename;

    public:

    download(
        const uri &         uri_,
        const std::string & filename)
    :
        m_uri(uri_),
        m_filename(filename)
    {}

    bool operator () () const { return run(nullptr); }

    bool operator () (online_data_processor & processor) const {
        return run(&processor);
    }

    private:

    bool run(online_data_processor * processor) const;

};  // end of class download

}  // end of namespace libfastcrawl

#endif  // end of #ifndef fastcrawl__download_hxx
