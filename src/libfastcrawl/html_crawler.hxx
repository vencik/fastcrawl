#ifndef fastcrawl__html_crawler_hxx
#define fastcrawl__html_crawler_hxx

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

#include "online_data_processor.hxx"
#include "thread_pool.hxx"
#include "logger.hxx"

#include <unordered_map>
#include <iostream>
#include <cassert>
#include <cstdint>


namespace fastcrawl {

/**
 *  \brief  HTML contengt reference attribute crawler
 *
 *  This is a simple, speed-optimised, online HTML doc|tag|attribute segmenter.
 *  It's used to seek registered element attributes and provide their values.
 *
 *  Basically, the crawler is a simple Finite State Automaton with 3 top-level
 *  nodes in line (document <-> tag <-> attribute).
 *  Within these nodes, further parsing goes character by character
 *  (with minimal rule set).
 *  It also supports faster skipping of uninteresting element tags
 *  (their attributes are not parsed).
 *  At any point, the processing may be interrupted and continued (when
 *  further data become available).
 *
 *  When a registered element attribute is found (content URI), it's downloaded.
 *  The crawler executes the download in separate thread from a thread pool.
 *  The download also computes Adler32 checksum and collects the total content
 *  size online.
 *  The results are stored in a record and may be reported eventually.
 *
 *  NOTE: The implementation is far from being perfect.
 *  It should be considered more a draft or proof of concept.
 *  It might need to be replaced with a proper XML/HTML online parser
 *  for serious applications.
 */
class html_crawler: public online_data_processor, public logger {
    private:

    /** Content download record */
    struct uri_record {
        std::string filename;   /**< Content storage file name */
        uint32_t    adler32;    /**< Content Adler32 checksum  */
        size_t      size;       /**< Content size              */
        bool        success;    /**< Content download status   */

        uri_record():
            adler32(0),
            size(0),
            success(false)
        {}

    };  // end of struct uri_record

    /** Map of URI -> content download records */
    using uri_records_t = std::unordered_map<std::string, uri_record>;

    /** Map of registered element attributes bearing content URI */
    class attribute_map: public std::unordered_map<std::string, std::string> {
        public:

        attribute_map();

    };  // end of class attribute_map

    /**
     *  \brief  Crawler FSA node
     *
     *  Abstract ancestor of the doc|tag|attribute FSA nodes.
     */
    struct html_node {
        html_crawler & crawler;     /**< Crawler reference */
        bool           done;        /**< Parsing done flag */

        html_node(html_crawler & crawler_):
            crawler(crawler_),
            done(false)
        {}

        /**
         *  \brief  Parsing entrypoint
         *
         *  When entering the respective FSA node, this function implements
         *  parsing of the \c data chunk (beginning at \c offset).
         *
         *  \param  data    Data chunk
         *  \param  size    Data chunk size
         *  \param  offset  Current offset in data chunk
         */
        virtual void crawl(unsigned char * data, size_t size, size_t & offset) = 0;

        virtual ~html_node() {}

    };  // end of struct html_node

    /**
     *  \brief  Crawler FSA document-level node
     *
     *  Implements crawling outside of a tag.
     */
    struct html_doc: public html_node {
        html_doc(html_crawler & crawler):
            html_node(crawler)
        {}

        /** Switch to FSA tag-level node */
        void descend() {
            crawler.m_current_node = &crawler.m_tag;
        }

        /** Implements \ref html_node::crawl */
        void crawl(unsigned char * data, size_t size, size_t & offset);

    };  // end of struct html_doc

    /**
     *  \brief  Crawler FSA tag-level node
     *
     *  Implements crawling within an element tag (opening or closing)
     *  or indeed other tag-syntax metadata (like comments etc).
     *
     *  The tag parsing is as permissive as possible.
     *
     *  NOTE: This part is the fishier; probably needs much more work.
     */
    struct html_tag: public html_node {
        bool          close;            /**< Closing tag flag                 */
        bool          skipped;          /**< Skipped tag (simplified parsing) */
        bool          name_done;        /**< Element name parsing done        */
        bool          comment;          /**< Comment tag (implies skipped)    */
        bool          comment_begin;    /**< Comment beginning state          */
        bool          comment_end;      /**< Comment ending state             */
        unsigned char last_ch;          /**< Last character cache             */
        std::string   name;             /**< Element name                     */

        /**
         *  In case the tag is an opeining of a registered element,
         *  \c seek_attr points to the registered attribute.
         */
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

        /** Switch back to document-level FSA node */
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

        /** Switch to attribute-level FSA node */
        void descend(unsigned char ch) {
            crawler.m_element_attr.name += ch;

            crawler.m_current_node = &crawler.m_element_attr;
        }

        /** Implements \ref html_node::crawl */
        void crawl(unsigned char * data, size_t size, size_t & offset);

        /** Implements \ref html_node::crawl for skipped tags */
        void crawl_skipped(unsigned char * data, size_t size, size_t & offset);

        /** Implements \ref html_node::crawl for non-skipped tags */
        void crawl_attrs(unsigned char * data, size_t size, size_t & offset);

    };  // end of struct html_tag

    /**
     *  \brief  Crawler FSA element attribute level node
     *
     *  Implements crawling within an element tag attribute.
     *
     *  Extracts the value and calls \ref html_crawler::process_uri
     *  if the attribute is registered.
     *
     *  NOTE: This part is the fishier; probably needs much more work.
     */
    struct html_element_attribute: public html_node {
        bool          has_value;    /**< Attribute has value              */
        std::string   name;         /**< Attribute name                   */
        unsigned char quote;        /**< Quote character used for value   */
        size_t        line;         /**< Value line position in content   */
        size_t        column;       /**< Value column position on \c line */
        std::string   value;        /**< Collected attribute value        */

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

        /** Switch back to tag-level FSA node */
        void ascend() {
            has_value = false;
            quote     = '\0';
            line      = 0;
            column    = 0;
            name.clear();
            value.clear();

            crawler.m_current_node = &crawler.m_tag;
        }

        /** Implements \ref html_node::crawl */
        void crawl(unsigned char * data, size_t size, size_t & offset);

        /**
         *  \brief  Process attribute value
         *
         *  As soon as the attribute value is collected, this function
         *  checks if the attribute name matches the attribute registered
         *  for the current element.
         *  If so, \ref html_crawler::process_uri is called.
         */
        void process() const {
            assert(crawler.s_attribute_map.end() != crawler.m_tag.seek_attr);

            if (name == crawler.m_tag.seek_attr->second)
                crawler.process_uri(crawler.m_tag.name, name, value, line, column);
        }

    };  // end of struct html_element_attribute

    /** Map of registered element attributes (tag -> attribute) */
    static const attribute_map s_attribute_map;

    const std::string m_host;   /**< HTTP Host (for non-absolute URIs) */

    // Position in content
    size_t m_read_cnt;  /**< Read byte counter           */
    size_t m_line;      /**< Current content line number */
    size_t m_column;    /**< Current line column number  */

    // Segmentation
    html_doc                 m_doc;             /**< Document-level FSA node  */
    html_tag                 m_tag;             /**< Tag-level FSA node       */
    html_element_attribute   m_element_attr;    /**< Attribute-level FSA node */
    html_node              * m_current_node;    /**< Current FSA node         */

    // URI records
    uri_records_t m_uri_records;    /**< Collected download records */

    // Downloads
    thread_pool m_download_tp;  /**< Download thread pool */

    public:

    /**
     *  \brief  Constructor
     *
     *  \param  host                     HTTP Host (for non-absolute URIs)
     *  \param  parallel_download_limit  Max. amount of download threads
     */
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

    /** Implements \ref online_data_processor::operator() */
    void operator () (unsigned char * data, size_t size);

    /** Wait till all downloads have finished */
    void wait() { m_download_tp.shutdown(); }

    /**
     *  \brief  Report download results
     *
     *  Note that the download records are accessed from download treads
     *  unlocked.
     *  The download threads don't race on the download records, as each
     *  has its own.
     *
     *  However, calling this function before te downloads have finished
     *  would indeed be a race condition (not to mention that the data
     *  would probably not be consistent yet).
     *  Therefore, this function must only be called after \ref wait.
     */
    void report() const;

    private:

    /** Update content position */
    void update_position(unsigned char ch) {
        if ('\n' == ch) {
            ++m_line;
            m_column = 0;
        }
        else ++m_column;

        ++m_read_cnt;
    }

    /**
     *  \brief  Download content
     *
     *  The function implements the job for a download thread.
     *
     *  \param  uri_str  Content URI
     *  \param  line     URI line position in crawled HTML code
     *  \param  column   URI column position on \c line
     *  \param  record   Download record for the job results
     */
    void download(
        const std::string & uri_str,
        size_t              line,
        size_t              column,
        uri_record &        record);

    /**
     *  \brief  Process found content URI reference
     *
     *  Filters out local anchors.
     *  Creates new download record and pushes download job to thread pool
     *  job queue.
     *
     *  \param  element_name    Element name
     *  \param  attribute_name  Attribute name
     *  \param  uri_str         Attribute value (content URI)
     *  \param  line            Value line position in crawled HTML code
     *  \param  column          Value column position on \c line
     *
     */
    void process_uri(
        const std::string & element_name,
        const std::string & attribute_name,
        const std::string & uri_str,
        size_t              line,
        size_t              column);

    /** Download record serialisation */
    friend std::ostream & operator << (
        std::ostream &     out,
        const uri_record & rec);

};  // end of class html_crawler

}  // end of namespace fastcrawl

#endif  // and of #ifndef fastcrawl__html_crawler_hxx
