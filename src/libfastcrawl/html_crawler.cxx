#include "html_crawler.hxx"
#include "download.hxx"
#include "utility.hxx"

#include <iostream>
#include <iomanip>
#include <functional>
#include <sstream>
#include <cctype>


namespace fastcrawl {

html_crawler::attribute_map::attribute_map() {
    emplace("a",      "href");
    emplace("img",    "src");
    emplace("script", "src");
    emplace("iframe", "src");
}

const html_crawler::attribute_map html_crawler::s_attribute_map;


inline static bool token_char(unsigned char ch) {
    if ('a' <= ch && ch <= 'z') return true;
    if ('A' <= ch && ch <= 'Z') return true;
    if ('0' <= ch && ch <= '9') return true;
    if ('-' == ch || ':' == ch) return true;

    return false;
}


static void subdownload(
    const std::string & uri,
    size_t              line,
    size_t              column,
    const std::string & base_uri)
{
    std::stringstream filename;
    filename
        << "./"
        << std::setw(8) << std::setfill('0') << line << '_'
        << std::setw(8) << std::setfill('0') << column;

    std::cerr
        << "Downloading URI \"" << uri
        << "\", storing as " << filename.str()
        << std::endl;

    download(uri, filename.str())();
}


void html_crawler::process_uri(
    const std::string & element_name,
    const std::string & attribute_name,
    const std::string & uri,
    size_t              line,
    size_t              column)
{
    std::cerr
        << "Element "      << element_name
        << " attribute "   << attribute_name
        << " URI: \""      << uri << "\""
        << " at position " << line  << ":" << column
        << std::endl;

    const auto iter_new = m_uri_set.insert(uri);
    if (iter_new.second) {
        m_download_tp.run(std::bind(&subdownload,
            uri, line, column, m_base_uri));

        std::cerr << "DOWNLOAD QUEUED" << std::endl;
    }
}


void html_crawler::operator () (unsigned char * data, size_t size) {
    size_t offset = 0;
    while (offset < size)
        m_current_node->crawl(data, size, offset);
}


void html_crawler::html_doc::crawl(
    unsigned char * data,
    size_t          size,
    size_t        & offset)
{
    while (offset < size) {
        const unsigned ch = data[offset++];
        crawler.update_position(ch);

        switch (ch) {
            // Element begin
            case '<':
                descend();
                return;

            // Not interesting
            default:
                break;
        }
    }
}


void html_crawler::html_tag::crawl(
    unsigned char * data,
    size_t          size,
    size_t        & offset)
{
    if (skipped)
        crawl_skipped(data, size, offset);
    else
        crawl_attrs(data, size, offset);
}


void html_crawler::html_tag::crawl_skipped(
    unsigned char * data,
    size_t          size,
    size_t        & offset)
{
    while (offset < size) {
        const unsigned ch = data[offset++];
        crawler.update_position(ch);

        run_at_eos(([ch, this]() { last_ch = ch; }));

        switch (ch) {
            // End of tag
            case '>':
                if (!comment || comment_end) {
                    ascend();
                    return;
                }

                break;

            case '-':
                if (comment_begin) {
                    if ('-' == last_ch) comment = true;
                }
                else if (comment) {
                    if ('-' == last_ch) comment_end = true;
                }

                break;

            default:
                comment_begin = false;
                break;
        }
    }
}


void html_crawler::html_tag::crawl_attrs(
    unsigned char * data,
    size_t          size,
    size_t        & offset)
{
    while (offset < size) {
        const unsigned ch = data[offset++];
        crawler.update_position(ch);

        run_at_eos(([ch, this]() { last_ch = ch; }));

        switch (ch) {
            // Element ends
            case '>':
                ascend();
                return;

            // Comment or metadata (or syntax error)
            case '!':
                comment_begin = name.empty();
                // Intentional fall through

            case '?':
                skipped = true;
                return;

            // Closed
            case '/':
                close = true;
                break;

            // Whitespace
            case ' ':
            case '\r':
            case '\n':
            case '\t':
                if (!name.empty()) {
                    name_done = true;

                    // Got element name, check if it's interesting
                    seek_attr = crawler.s_attribute_map.find(name);
                    if (crawler.s_attribute_map.end() == seek_attr) {
                        skipped = true;  // not an interesting element
                        return;
                    }
                }

                break;

            // Dash might be part of the name
            case '-':
                if (!name_done && !name.empty()) name += std::tolower(ch);

                // Syntax error
                else {
                    skipped = true;
                    return;
                }

                break;

            // Name or attribute definition
            default:
                if (token_char(ch)) {
                    // Part of the name
                    if (!name_done) name += std::tolower(ch);

                    // Attribute begins
                    else {
                        descend(ch);
                        return;
                    }
                }

                // Syntax error
                else {
                    skipped = true;
                    return;
                }

                break;
        }
    }
}


void html_crawler::html_element_attribute::crawl(
    unsigned char * data,
    size_t          size,
    size_t        & offset)
{
    while (offset < size) {
        const unsigned ch = data[offset++];
        crawler.update_position(ch);

        switch (ch) {
            // Element ends
            case '/':
                if ('\0' == quote) {
                    crawler.m_tag.close = true;
                    ascend();
                    return;
                }
                else value += ch;  // part of value

                break;

            // Element ends
            case '>':
                process();
                ascend();
                crawler.m_tag.ascend();
                return;

            // Value assignment
            case '=':
                has_value = true;
                break;

            // Value begin/end
            case '\'':
            case '"':
                // Value begins
                if ('\0' == quote) {
                    quote  = ch;
                    line   = crawler.m_line;
                    column = crawler.m_column;
                }

                // We're done
                else if (quote == ch) {
                    process();
                    ascend();
                    return;
                }

                else value += ch;  // part of value

                break;

            // Whitespace
            case ' ':
            case '\r':
            case '\n':
            case '\t':
                if ('\0' != quote) value += ch;  // part of value

                break;

            // Accumulate name or value
            default:
                if ('\0' != quote) value += ch;
                else               name  += std::tolower(ch);

                break;
        }
    }
}

}  // end of namespace fastcrawl
