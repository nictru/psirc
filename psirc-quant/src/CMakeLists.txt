file(GLOB sources *.cpp)
file(GLOB headers *.h *.hpp)

list(REMOVE_ITEM sources main.cpp)

add_library(psirc_core ${sources} ${headers})
target_include_directories(psirc_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(psirc-quant main.cpp)

find_package( Threads REQUIRED )
target_link_libraries(psirc-quant psirc_core pthread z)

if(LINK MATCHES static)
    set(BUILD_SHARED_LIBS OFF)
    set(HDF5_USE_STATIC_LIBRARIES 1)

    if (UNIX AND NOT APPLE)
        #set(CMAKE_EXE_LINKER_FLAGS  "-static -static-libgcc -static-libstdc++")
        set(CMAKE_EXE_LINKER_FLAGS  "-static -static-libstdc++")
        SET(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
        set(CMAKE_EXE_LINKER_FLAGS  "-static -static-libgcc -static-libstdc++")
    endif(UNIX AND NOT APPLE)

    SET_TARGET_PROPERTIES(psirc-quant psirc_core PROPERTIES LINK_SEARCH_END_STATIC 1)
endif(LINK MATCHES static)

find_package( HDF5 REQUIRED )
find_package( ZLIB REQUIRED )

if ( ZLIB_FOUND )
    include_directories( ${ZLIB_INCLUDE_DIRS} )
else()
    message(FATAL_ERROR "zlib not found. Required for to output files" )
endif( ZLIB_FOUND )

if(HDF5_FOUND)
    include_directories( ${HDF5_INCLUDE_DIRS} )
    target_link_libraries( psirc_core ${HDF5_LIBRARIES} )
    target_link_libraries( psirc-quant ${HDF5_LIBRARIES} )
else()
    message(FATAL_ERROR "HDF5 not found. Required to output files")
endif()


find_library(HTSLIB
    NAMES hts
    REQUIRED
)

target_link_libraries(psirc-quant ${HTSLIB})

if(LINK MATCHES static)
    if (UNIX AND NOT APPLE)
        target_link_libraries(psirc-quant librt.a)
    endif()
else()
    if (UNIX AND NOT APPLE)
        target_link_libraries(psirc-quant rt)
    endif()
endif(LINK MATCHES static)


install(TARGETS psirc-quant DESTINATION "${CMAKE_INSTALL_BINDIR}")
