cmake_minimum_required(VERSION 3.12)

cmake_policy(VERSION 3.12)
if (POLICY CMP0076)
    cmake_policy(SET CMP0076 NEW)
endif()

add_compile_options(/Dstrncasecmp=_strnicmp /Dstrcasecmp=_stricmp /DWIN32 /D__WIN32__)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_EXE_LINKER_FLAGS    "${CMAKE_EXE_LINKER_FLAGS} /MANIFEST:NO")
set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} /MANIFEST:NO")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /MANIFEST:NO")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /CLRTHREADATTRIBUTE:STA")


if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

if (CMAKE_BUILD_TYPE STREQUAL Release)
	option(ENABLE_LTO "Enable link-time optimization" ON)
elseif (CMAKE_BUILD_TYPE STREQUAL RelWithDebInfo)
	option(ENABLE_LTO "Enable link-time optimization" ON)
else()
	option(ENABLE_LTO "Enable link-time optimization" OFF)
endif()

if(ENABLE_LTO)
	add_compile_options(/EHsc /GL /O2)
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /LTCG")
	set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} /LTCG")
	set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /LTCG")
endif()

#add_compile_options(-fno-pic)
#add_link_options(/LTCG)

option(BUILD_LIBUI "Build libui frontend" ON)

add_subdirectory(src)

if (BUILD_LIBUI)
	add_subdirectory(src/libui_sdl)
endif()

configure_file(
	${CMAKE_SOURCE_DIR}/romlist.bin
	${CMAKE_BINARY_DIR}/romlist.bin COPYONLY)
	
project(melonDS)