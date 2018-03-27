/**
 *  \file
 *  \brief  HTML crawler
 *
 *  \date   2018/03/27
 *  \author Vaclav Krpec  <vencik@razdva.cz>
 *
 *
 *  LEGAL NOTICE
 *
 *  Copyright (c) 2018, Vaclav Krpec
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *
 *  1. Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the distribution.
 *
 *  3. Neither the name of the copyright holder nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 *  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 *  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER
 *  OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 *  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 *  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 *  OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 *  WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 *  OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 *  EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "html_crawler.hxx"
#include "adler32.hxx"
#include "content_size.hxx"
#include "download.hxx"
#include "utility.hxx"
#include "uri.hxx"

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


/** Token character check */
inline static bool token_char(unsigned char ch) {
    if ('a' <= ch && ch <= 'z') return true;
    if ('A' <= ch && ch <= 'Z') return true;
    if ('0' <= ch && ch <= '9') return true;
    if ('-' == ch || ':' == ch) return true;

    return false;
}


void html_crawler::download(
    const std::string &        uri_str,
    size_t                     line,
    size_t                     column,
    html_crawler::uri_record & record)
{
    std::stringstream filename_ss; filename_ss
        << "./"
        << std::setw(8) << std::setfill('0') << line << '_'
        << std::setw(8) << std::setfill('0') << column;

    record.filename = filename_ss.str();

    auto uri = uri::parse(uri_str);
    if (uri.host.empty()) uri.host = m_host;  // fix relative URIs

    // Data processors
    auto dproc = data_processor(
        adler32(record.adler32),
        content_size(record.size));

    fastcrawl::download dl(uri, filename_ss.str());

    dl.verbose_log(verbose_log());  // set logging

    record.success = dl(dproc);  // sub-download with Adler32 checksum
}


void html_crawler::process_uri(
    const std::string & element_name,
    const std::string & attribute_name,
    const std::string & uri_str,
    size_t              line,
    size_t              column)
{
    VLOG
       << "Element "      << element_name
       << " attribute "   << attribute_name
       << " URI: \""      << uri_str << "\""
       << " at position " << line  << ":" << column
       << std::endl;

    // Ommit local fragment ref
    if (!uri_str.empty() && '#' == uri_str[0]) return;

    const auto iter_new = m_uri_records.emplace(uri_str, uri_record());
    if (iter_new.second) {
        m_download_tp.run(std::bind(&html_crawler::download,
            this, uri_str, line, column, std::ref(iter_new.first->second)));
    }
}


std::ostream & operator << (
    std::ostream &                   out,
    const html_crawler::uri_record & rec)
{
    const auto cout_flags = out.flags();

    out << rec.filename
        << " size: " << std::dec << rec.size
        << ", Adler32 checksum: "
        << std::hex << std::setw(8) << std::setfill('0')
        << rec.adler32;

    out.flags(cout_flags);

    return out;
}


void html_crawler::report() const {
    const uri_record * min_size_rec = nullptr;
    const uri_record * max_size_rec = nullptr;

    for (auto & uri_record: m_uri_records) {
        const auto & uri = uri_record.first;
        const auto & rec = uri_record.second;

        std::cout << "URI \"" << uri << "\" stored in " << rec << std::endl;

        if (!min_size_rec || min_size_rec->size > rec.size)
            min_size_rec = &rec;

        if (!max_size_rec || max_size_rec->size < rec.size)
            max_size_rec = &rec;
    }

    if (min_size_rec)
        std::cout << "Minimal size: " << *min_size_rec << std::endl;

    if (max_size_rec)
        std::cout << "Maximal size: " << *max_size_rec << std::endl;
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
