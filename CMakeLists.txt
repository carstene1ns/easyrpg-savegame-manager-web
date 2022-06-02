cmake_minimum_required(VERSION 3.13 FATAL_ERROR)
project(EasyRPG-SMW
	VERSION 1.0)

#inih
add_library(inih
	src/ini.c
	src/ini.h)

# configure nanogui
set(NANOGUI_BUILD_EXAMPLES OFF CACHE BOOL " " FORCE)
set(NANOGUI_BUILD_PYTHON   OFF CACHE BOOL " " FORCE)
set(NANOGUI_INSTALL        OFF CACHE BOOL " " FORCE)
set(NANOGUI_BUILD_GLFW     OFF CACHE BOOL " " FORCE)
add_subdirectory(ext/nanogui)

add_definitions(${NANOGUI_EXTRA_DEFS})
include_directories(${NANOGUI_EXTRA_INCS})

add_executable(ESMW src/main.cpp)
target_link_libraries(ESMW inih nanogui ${NANOGUI_EXTRA_LIBS})
set_property(TARGET ESMW PROPERTY CXX_STANDARD 17)

if(EMSCRIPTEN)
	target_link_libraries(ESMW idbfs.js)
	set_property(TARGET ESMW PROPERTY LINK_OPTIONS
		"-sUSE_GLFW=3"
		"-sASYNCIFY"
		"-O3")
	set(CMAKE_EXECUTABLE_SUFFIX ".html")

	# debug
	set_property(TARGET ESMW APPEND PROPERTY LINK_OPTIONS
		"-sASSERTIONS=2"
		"-sALLOW_MEMORY_GROWTH=1"
		"-sDEMANGLE_SUPPORT"
		"-g")
endif()
