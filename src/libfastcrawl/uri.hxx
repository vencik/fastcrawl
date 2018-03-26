#ifndef fastcrawl__uri_hxx
#define fastcrawl__uri_hxx

#include <string>
#include <regex>
#include <cstdint>


namespace fastcrawl {

class uri {
    private:

    static const std::regex s_regex;

    public:

    std::string scheme;
    std::string user;
    std::string password;
    std::string authority;
    uint16_t    port;
    std::string path;
    std::string query;
    std::string fragment;

    uri():
        port(0)
    {}

    uri(const std::string & scheme_,
        const std::string & user_,
        const std::string & password_,
        const std::string & authority_,
        uint16_t            port_,
        const std::string & path_,
        const std::string & query_,
        const std::string & fragment_)
    :
        scheme(scheme_),
        user(user_),
        password(password_),
        authority(authority_),
        port(port_),
        path(path_),
        query(query_),
        fragment(fragment_)
    {}

    static uri parse(const std::string & uri_);

    bool operator == (const uri & arg) const;

    bool operator != (const uri & arg) const { return !((*this) == arg); }

    operator std::string () const;

};  // end of class uri

}  // end of namespace fastcrawl

#endif  // and of #ifndef fastcrawl__uri_hxx
