cmake_minimum_required(VERSION 3.13 FATAL_ERROR)
project(EasyRPG-SMW
	VERSION 1.0)

# configure nanogui
set(NANOGUI_BUILD_EXAMPLES OFF CACHE BOOL " " FORCE)
set(NANOGUI_BUILD_PYTHON   OFF CACHE BOOL " " FORCE)
set(NANOGUI_INSTALL        OFF CACHE BOOL " " FORCE)
set(NANOGUI_BUILD_GLFW     OFF CACHE BOOL " " FORCE)
add_subdirectory(ext/nanogui)

add_definitions(${NANOGUI_EXTRA_DEFS})
include_directories(${NANOGUI_EXTRA_INCS})

add_executable(ESMW src/main.cpp)
target_link_libraries(ESMW nanogui ${NANOGUI_EXTRA_LIBS})
set_property(TARGET ESMW PROPERTY CXX_STANDARD 17)

if(EMSCRIPTEN)
	target_link_libraries(ESMW idbfs.js)
	set_property(TARGET ESMW PROPERTY LINK_OPTIONS "-s USE_GLFW=3")
	set(CMAKE_EXECUTABLE_SUFFIX ".html")

	# debug
	set_property(TARGET ESMW APPEND PROPERTY LINK_OPTIONS
		"-s ASSERTIONS=2 -s ALLOW_MEMORY_GROWTH=1 -s DEMANGLE_SUPPORT -g")
endif()
