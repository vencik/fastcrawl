add_library(fastcrawl
    download.cxx
    html_crawler.cxx
    thread_pool.cxx
    uri.cxx
    adler32.cxx
    content_size.cxx
)
target_link_libraries(fastcrawl
    LINK_PUBLIC pthread
    LINK_PUBLIC curl
    LINK_PUBLIC z)
