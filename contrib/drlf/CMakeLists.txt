check_include_file_cxx(stdint.h HAVE_STDINT_H)
if(HAVE_STDINT_H)
    add_definitions(-DHAVE_STDINT_H)
endif()

set(examples_as_tests_sources)
if(${ENABLE_EXAMPLES})
    set(examples_as_tests_sources
        #test/drlf-examples-test-suite.cc
        )
endif()

build_lib(
    LIBNAME drlf
    SOURCE_FILES model/drlf-routing.cc
                 model/drlf-table.cc
                 model/drlf-tag.cc
                 model/drlf-conf.cc
                 model/drlf-util.cc
                 helper/drlf-helper.cc
    HEADER_FILES model/drlf-routing.h
                 model/drlf-table.h
                 model/drlf-tag.h
                 model/drlf-conf.h
                 model/drlf-util.h
                 helper/drlf-helper.h
    LIBRARIES_TO_LINK
        ${libcore}
        ${libsatellite}
        ${ns3-libs}
        ${ns3-contrib-libs}
    TEST_SOURCES test/drlf-test-suite.cc
                 ${examples_as_tests_sources}
)

