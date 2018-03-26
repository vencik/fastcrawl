# URI
add_executable(ut_uri uri.cxx)
target_link_libraries(ut_uri LINK_PUBLIC fastcrawl)

add_test(URI ut_uri)
#    NAME    URI
#    COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/levenshtein.sh"
#        "${CMAKE_SOURCE_DIR}" "${CMAKE_BINARY_DIR}")
