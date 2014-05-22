cmake_minimum_required(VERSION 2.8)

project(testSphinx)

include(FindPkgConfig)

set(ENV{LD_LIBRARY_PATH} "/usr/local/lib")
set(ENV{PKG_CONFIG_PATH} "/usr/local/lib/pkgconfig")

execute_process(COMMAND pkg-config --variable=modeldir pocketsphinx OUTPUT_VARIABLE MODELDIR)
string(REGEX REPLACE "(\r?\n)+$" "" MODELDIR "${MODELDIR}")
set(MODELDIR "\"${MODELDIR}\"")
add_definitions(-DMODELDIR=${MODELDIR})

pkg_check_modules(sphinx REQUIRED pocketsphinx sphinxbase)

if (sphinx_FOUND)
    include_directories(${sphinx_INCLUDE_DIRS})
    link_directories(${sphinx_LIBRARY_DIRS})
    set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} ${sphinx_CFLAGS})

    add_executable(recordCommand "src/recordCommand.c")
    target_link_libraries(recordCommand ${sphinx_LIBRARIES})
    
    add_executable(testSphinx "src/testSphinx.c")
    target_link_libraries(testSphinx ${sphinx_LIBRARIES})
    
    add_executable(onButtonSphinx "src/onButtonSphinx.c")
    target_link_libraries(onButtonSphinx ${sphinx_LIBRARIES})
endif (sphinx_FOUND)