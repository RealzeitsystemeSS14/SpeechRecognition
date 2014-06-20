cmake_minimum_required(VERSION 2.8)

project(testSphinx)

include(FindPkgConfig)

set(ENV{LD_LIBRARY_PATH} "/usr/local/lib")
set(ENV{PKG_CONFIG_PATH} "/usr/local/lib/pkgconfig")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/modules")

execute_process(COMMAND pkg-config --variable=modeldir pocketsphinx OUTPUT_VARIABLE MODELDIR)
string(REGEX REPLACE "(\r?\n)+$" "" MODELDIR "${MODELDIR}")
set(MODELDIR "\"${MODELDIR}\"")
add_definitions(-DMODELDIR=${MODELDIR} -DTAKE_TIME -D_GNU_SOURCE)

pkg_check_modules(sphinx REQUIRED pocketsphinx sphinxbase)
find_package(Allegro REQUIRED)

if (sphinx_FOUND AND ALLEGRO_FOUND)
    include_directories(${sphinx_INCLUDE_DIRS} ${ALLEGRO_INCLUDE_DIR})
    link_directories(${sphinx_LIBRARY_DIRS}})
    set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} ${sphinx_CFLAGS})

    add_executable(recordCommand "src/test/recordCommand.c")
    target_link_libraries(recordCommand ${sphinx_LIBRARIES})
    
    add_executable(testSphinx "src/test/testSphinx.c")
    target_link_libraries(testSphinx ${sphinx_LIBRARIES})
    
    add_executable(onButtonSphinx "src/test/onButtonSphinx.c")
    target_link_libraries(onButtonSphinx ${sphinx_LIBRARIES})
    
    file(GLOB RTSPEECH_SRC "src/*.c" "src/*.h")
    add_executable(RTSpeech ${RTSPEECH_SRC})
    target_link_libraries(RTSpeech ${sphinx_LIBRARIES} ${ALLEGRO_LIBRARIES})
endif (sphinx_FOUND AND ALLEGRO_FOUND)