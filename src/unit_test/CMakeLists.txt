# URI
add_executable(ut_uri uri.cxx)
target_link_libraries(ut_uri LINK_PUBLIC fastcrawl)
add_test(URI ut_uri)


# Adler32
add_executable(ut_adler32 adler32.cxx)
target_link_libraries(ut_adler32
    LINK_PUBLIC fastcrawl
    LINK_PUBLIC z)
add_test(Adler32 ut_adler32)
