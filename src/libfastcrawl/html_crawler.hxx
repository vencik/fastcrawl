#ifndef fastcrawl__html_crawler_hxx
#define fastcrawl__html_crawler_hxx

#include "online_data_processor.hxx"
#include "thread_pool.hxx"
#include "logger.hxx"

#include <unordered_map>
#include <iostream>
#include <cassert>
#include <cstdint>


namespace fastcrawl {

class html_crawler: public online_data_processor, public logger {
    private:

    struct uri_record {
        std::string filename;
        uint32_t    adler32;
        size_t      size;
        bool        success;

        uri_record():
            adler32(0),
            size(0),
            success(false)
        {}

    };  // end of struct uri_record

    using uri_records_t = std::unordered_map<std::string, uri_record>;

    class attribute_map: public std::unordered_map<std::string, std::string> {
        public:

        attribute_map();

    };  // end of class attribute_map

    struct html_node {
        html_crawler & crawler;
        bool           done;

        html_node(html_crawler & crawler_):
            crawler(crawler_),
            done(false)
        {}

        virtual void crawl(unsigned char * data, size_t size, size_t & offset) = 0;

        virtual ~html_node() {}

    };  // end of struct html_node

    struct html_doc: public html_node {
        html_doc(html_crawler & crawler):
            html_node(crawler)
        {}

        void descend() {
            crawler.m_current_node = &crawler.m_tag;
        }

        void crawl(unsigned char * data, size_t size, size_t & offset);

    };  // end of struct html_doc

    struct html_tag: public html_node {
        bool          close;
        bool          skipped;
        bool          name_done;
        bool          comment;
        bool          comment_begin;
        bool          comment_end;
        unsigned char last_ch;
        std::string   name;

        attribute_map::const_iterator seek_attr;

        html_tag(html_crawler & crawler):
            html_node(crawler),
            close(false),
            skipped(false),
            name_done(false),
            comment(false),
            comment_begin(false),
            comment_end(false),
            last_ch('\0'),
            seek_attr(crawler.s_attribute_map.end())
        {
            name.reserve(64);  // reasonable element name length
        }

        void ascend() {
            close         = false;
            skipped       = false;
            name_done     = false;
            comment       = false;
            comment_begin = false;
            comment_end   = false;
            last_ch       = '\0';
            seek_attr     = crawler.s_attribute_map.end();
            name.clear();

            crawler.m_current_node = &crawler.m_doc;
        }

        void descend(unsigned char ch) {
            crawler.m_element_attr.name += ch;

            crawler.m_current_node = &crawler.m_element_attr;
        }

        void crawl(unsigned char * data, size_t size, size_t & offset);

        void crawl_skipped(unsigned char * data, size_t size, size_t & offset);

        void crawl_attrs(unsigned char * data, size_t size, size_t & offset);

    };  // end of struct html_tag

    struct html_element_attribute: public html_node {
        bool          has_value;
        std::string   name;
        unsigned char quote;
        size_t        line;
        size_t        column;
        std::string   value;

        html_element_attribute(html_crawler & crawler):
            html_node(crawler),
            has_value(false),
            quote('\0'),
            line(0),
            column(0)
        {
            name.reserve(128);    // reasonable attribute name length
            value.reserve(1024);  // reasonable attribute value length
        }

        void ascend() {
            has_value = false;
            quote     = '\0';
            line      = 0;
            column    = 0;
            name.clear();
            value.clear();

            crawler.m_current_node = &crawler.m_tag;
        }

        void crawl(unsigned char * data, size_t size, size_t & offset);

        void process() const {
            assert(crawler.s_attribute_map.end() != crawler.m_tag.seek_attr);

            if (name == crawler.m_tag.seek_attr->second)
                crawler.process_uri(crawler.m_tag.name, name, value, line, column);
        }

    };  // end of struct html_element_attribute

    static const attribute_map s_attribute_map;

    const std::string m_host;

    // Position in content
    size_t m_read_cnt;
    size_t m_line;
    size_t m_column;

    // Segmentation
    html_doc                 m_doc;
    html_tag                 m_tag;
    html_element_attribute   m_element_attr;
    html_node              * m_current_node;

    // URI records
    uri_records_t m_uri_records;

    // Downloads
    thread_pool m_download_tp;

    public:

    html_crawler(
        const std::string & host,
        size_t              parallel_download_limit = SIZE_MAX)
    :
        m_host(host),
        m_read_cnt(0),
        m_line(1),
        m_column(0),
        m_doc(*this),
        m_tag(*this),
        m_element_attr(*this),
        m_current_node(&m_doc),
        m_download_tp(20, parallel_download_limit)
    {}

    void operator () (unsigned char * data, size_t size);

    void wait() { m_download_tp.shutdown(); }

    void report() const;

    private:

    void update_position(unsigned char ch) {
        if ('\n' == ch) {
            ++m_line;
            m_column = 0;
        }
        else ++m_column;

        ++m_read_cnt;
    }

    void download(
        const std::string & uri_str,
        size_t              line,
        size_t              column,
        uri_record &        record);

    void process_uri(
        const std::string & element_name,
        const std::string & attribute_name,
        const std::string & uri_str,
        size_t              line,
        size_t              column);

    friend std::ostream & operator << (
        std::ostream &     out,
        const uri_record & rec);

};  // end of class html_crawler

}  // end of namespace fastcrawl

#endif  // and of #ifndef fastcrawl__html_crawler_hxx
