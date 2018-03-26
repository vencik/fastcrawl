#include "libfastcrawl/uri.hxx"

#include <iostream>
#include <list>


class uri_parser_test {
    private:

    struct test_case {
        const std::string    uri_str;
        const fastcrawl::uri uri;

        test_case(
            const std::string uri_str_,
            fastcrawl::uri && uri_)
        :
            uri_str(uri_str_),
            uri(uri_)
        {}

    };  // end of struct test_case

    std::list<test_case> m_test_cases;

    public:

    uri_parser_test() {
        m_test_cases.emplace_back(
            "www.meetangee.com",
            fastcrawl::uri("", "", "", "www.meetangee.com", 0, "", "", "")
        );
        m_test_cases.emplace_back(
            "https://github.com/vencik",
            fastcrawl::uri("https", "", "", "github.com", 0, "/vencik", "", "")
        );
        m_test_cases.emplace_back(
            "http://bob:secret@webproxy.example.com:8080/",
            fastcrawl::uri("http", "bob", "secret", "webproxy.example.com", 8080, "/", "", "")
        );
        m_test_cases.emplace_back(
            "https://www.example.com:8443/my/path/some.js?abc=123#whatever",
            fastcrawl::uri("https", "", "", "www.example.com", 8443, "/my/path/some.js", "abc=123", "whatever")
        );
        m_test_cases.emplace_back(
            "#whatever",
            fastcrawl::uri("", "", "", "", 0, "", "", "whatever")
        );
    }

    bool operator () () const {
        size_t test_cnt = 0;
        size_t fail_cnt = 0;

        for (auto & test_case: m_test_cases) {
            ++test_cnt;

            const auto uri = fastcrawl::uri::parse(test_case.uri_str);
            if (uri != test_case.uri) {
                std::cerr
                    << "URI parsing failed for \"" << test_case.uri_str << "\""
                    << std::endl
                    << "\texpected \"" << (std::string)test_case.uri << "\""
                    << std::endl
                    << "\tgot      \"" << (std::string)uri << "\""
                    << std::endl;

                ++fail_cnt;
            }
        }

        std::cerr
            << "URI parsing UT: "
            << fail_cnt << "/" << test_cnt << " failed"
            << std::endl;

        return 0 == fail_cnt;
    }

};  // end of class uri_parser_test

static const uri_parser_test uri_parser_ut;


// Actual main implementation
static int main_impl(int argc, char * const argv[]) {
    return uri_parser_ut() ? 0 : 1;
}


// Exception safeness wrapper
int main(int argc, char * const argv[]) {
    try {
        return main_impl(argc, argv);
    }

    // Unhandled standard exception
    catch (const std::exception & ex) {
        std::cerr
            << "Stndard exception caught: " << ex.what()
            << std::endl;
    }

    // Unhandled unknown exception
    catch (...) {
        std::cerr
            << "Unknown exception caught"
            << std::endl;
    }

    return 64;  // exception caught
}
