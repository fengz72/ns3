check_include_file_cxx(stdint.h HAVE_STDINT_H)
if(HAVE_STDINT_H)
    add_definitions(-DHAVE_STDINT_H)
endif()

set(examples_as_tests_sources)
if(${ENABLE_EXAMPLES})
    set(examples_as_tests_sources
        #test/dra-examples-test-suite.cc
        )
endif()

build_lib(
    LIBNAME dra
    SOURCE_FILES model/dra-routing.cc
                 model/dra-config.cc
                 model/dra-table-entry.cc
                 model/dra-tag.cc
        model/dra-utils.cc
                 helper/dra-routing-helper.cc
    HEADER_FILES model/dra-routing.h
                 model/dra-config.h
                 model/dra-table-entry.h
                 model/dra-tag.h
        model/dra-utils.h
                 helper/dra-routing-helper.h
    LIBRARIES_TO_LINK
        ${libcore}
        ${libsatellite}
        ${ns3-libs}
        ${ns3-contrib-libs}
    TEST_SOURCES test/dra-test-suite.cc
                 ${examples_as_tests_sources}
)

