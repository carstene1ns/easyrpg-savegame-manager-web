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
	set_target_properties(ESMW PROPERTIES
		LINK_FLAGS "-s EXPORTED_RUNTIME_METHODS=['ccall','cwrap'] --bind -s MALLOC=emmalloc -s WASM_BIGINT=1 -s USE_GLFW=3 -s ALLOW_MEMORY_GROWTH=1")
	set(CMAKE_EXECUTABLE_SUFFIX ".html")
endif()
