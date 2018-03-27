#ifndef fastcrawl__logger_hxx
#define fastcrawl__logger_hxx

#include <iostream>


#define LOG std::cerr


#define VLOG if (!m_vlog); else std::cerr


namespace fastcrawl {

class logger {
    protected:

    bool m_vlog;  /**< Verbose logging flag */

    public:

    logger(): m_vlog(false) {}

    bool verbose_log() const { return m_vlog; }

    void verbose_log(bool verbose) { m_vlog = verbose; }

};  // end of class logger

}  // end of namespace fastcrawl

#endif  // end of #ifndef fastcrawl__logger_hxx
