check_include_file_cxx(stdint.h HAVE_STDINT_H)
if(HAVE_STDINT_H)
    add_definitions(-DHAVE_STDINT_H)
endif()

set(examples_as_tests_sources)
if(${ENABLE_EXAMPLES})
    set(examples_as_tests_sources
        #test/dsra-examples-test-suite.cc
        )
endif()

build_lib(
    LIBNAME dsra
    SOURCE_FILES model/dsra-routing.cc
                 model/dsra-config.cc
                 model/dsra-table-entry.cc
                 model/dsra-utils.cc
                 model/dsra-tag.cc
                 helper/dsra-helper.cc
    HEADER_FILES model/dsra-routing.h
                 model/dsra-config.h
                 model/dsra-table-entry.h
                 model/dsra-utils.h
                 model/dsra-tag.h
                 helper/dsra-helper.h
    LIBRARIES_TO_LINK
        ${libcore}
        ${libsatellite}
        ${ns3-libs}
        ${ns3-contrib-libs}
    TEST_SOURCES test/dsra-test-suite.cc
                 ${examples_as_tests_sources}
)

