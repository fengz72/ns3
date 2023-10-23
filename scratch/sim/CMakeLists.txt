# Create a custom library
add_library(
        scratch-sim-lib
        utils/utils.cc utils/utils.h
)

# Link the custom library to the scratch executable
build_exec(
  EXECNAME sim-main
  SOURCE_FILES dra-example.cc
  LIBRARIES_TO_LINK
        scratch-sim-lib
#        ${libcore}
#        ${libpoint-to-point}
#        ${libinternet}
#        ${libapplications}
#        ${libmobility}
#        ${libsatellite}
#        ${libdra}
#        ${libflow-monitor-module}
        ${ns3-libs}
        ${ns3-contrib-libs}
  # use "${ns3-libs}" "${ns3-contrib-libs}" in case you want to link to all
  # enabled modules
  EXECUTABLE_DIRECTORY_PATH ${CMAKE_OUTPUT_DIRECTORY}/scratch/sim
)
