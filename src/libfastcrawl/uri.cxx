#include "uri.hxx"

#include <iostream>
#include <sstream>
#include <cassert>


namespace fastcrawl {

// URI regex
const std::regex uri::s_regex(
    "^"
    "(([A-Za-z0-9]+)://)?"                  // 2:  scheme
    "(([A-Za-z0-9]+)(:([A-Za-z0-9]+))@)?"   // 4:  user, 6: password
    "([A-Za-z%0-9.-]+)?"                    // 7:  authority
    "(:([0-9]+))?"                          // 9:  port
    "([^?#]*)"                              // 10: path
    "(\\?([^#]*))?"                         // 12: query
    "(#(.*))?"                              // 14: fragment
    "$"
);


uri uri::parse(const std::string & uri_) {
    std::smatch bref;

    // Invalid URI
    if (!std::regex_match(uri_, bref, s_regex))
        return uri(
            std::string(),
            std::string(),
            std::string(),
            std::string(),
            0,
            std::string(),
            std::string(),
            std::string());

    // Port (defaults)
    uint16_t port = 0;
    if (!bref[9].str().empty()) {
        std::stringstream port_ss(bref[9]);
        port_ss >> port;
        assert(!port_ss.fail());
    }

    return uri(
        bref[2],
        bref[4],
        bref[6],
        bref[7],
        port,
        bref[10],
        bref[12],
        bref[14]);
}


bool uri::operator == (const uri & arg) const {
    return
        scheme    == arg.scheme     &&
        user      == arg.user       &&
        password  == arg.password   &&
        authority == arg.authority  &&
        port      == arg.port       &&
        path      == arg.path       &&
        query     == arg.query      &&
        fragment  == arg.fragment;
}


uri::operator std::string () const {
    std::stringstream uri_ss;

    uri_ss << (scheme.empty() ? "http" : scheme) << "://";  // mandatory

    if (!user.empty()) {
        uri_ss << user;
        if (!password.empty())
            uri_ss << ':' << password;
        uri_ss << '@';
    }

    uri_ss << authority;

    if (!authority.empty() && port) uri_ss << ':' << port;

    if (!path.empty()) {
        if (!authority.empty() && '/' != path[0]) uri_ss << '/';
        uri_ss << path;
    }

    if (!query.empty())    uri_ss << '?' << query;
    if (!fragment.empty()) uri_ss << '#' << fragment;

    return uri_ss.str();
}

}  // end of namespace fastcrawl
