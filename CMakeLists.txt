cmake_minimum_required (VERSION 3.16)


project( JJSON )


add_library(JJSON 
	"JJSON.cpp"
	"JJSON.h"
	"JJSON.hpp"
	"JJSONcommon.h"
	"NatvisFile.natvis"
	"framework.h"
	"pch.cpp"
	"pch.h"
)

target_precompile_headers( JJSON PRIVATE "pch.h" )
target_include_directories( JJSON PUBLIC ${CMAKE_SOURCE_DIR} )

if(MSVC)
    target_compile_options(JJSON PRIVATE /permissive- /Zc:inline /Zc:lambda /Zc:preprocessor /Zc:throwingNew ) # add all conformance options on msvc compiler
endif()

target_compile_features( JJSON PRIVATE cxx_std_17 )
set_target_properties( JJSON PROPERTIES 
    VISIBILITY_INLINES_HIDDEN true
    CXX_VISIBILITY_PRESET hidden
)