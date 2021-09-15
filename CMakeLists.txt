cmake_minimum_required(VERSION 3.8)
# cmake version 3.8 is required for CXX 17
project(shader-tester-qt)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(BUILD_SHARED_LIBS OFF)

include_directories("common-cpp")

# used this to add common-cpp as a submodule
# git submodule add ssh://foxfire@foxfire256.ddns.net:27730/nas/dev/common-cpp common-cpp
# then manually add the following to .gitmodules
# url = git@github.com:foxfire256/common-cpp.git

# git submodule add ssh://foxfire@foxfire256.ddns.net:27730/nas/dev/EpsilonEridaniOpenGL EpsilonEridaniOpenGL
# then manually add the following to .gitmodules
# url = git@github.com:foxfire256/EpsilonEridaniOpenGL.git

# git submodule add ssh://foxfire@foxfire256.ddns.net:27730/nas/dev/model-viewer-data data

set(DATA_ROOT "${CMAKE_CURRENT_SOURCE_DIR}")
#add_compile_definitions(DATA_ROOT="${DATA_ROOT}")
set(COMPILE_DEFINITIONS "${COMPILE_DEFINITIONS},DATA_ROOT=${CMAKE_CURRENT_SOURCE_DIR}")

# cmake .. -G "Visual Studio 16 2019" -A x64
# cmake .. -G "Visual Studio 17 2022" -A x64
if(MSVC)
	set(CMAKE_PREFIX_PATH "C:\\usr\\vclibs")
	set(CMAKE_INCLUDE_PATH "C:\\usr\\vclibs\\include")
	set(CMAKE_LIBRARY_PATH "C:\\usr\\vclibs\\lib")
	
	SET(CMAKE_C_FLAGS "/MP ${CMAKE_C_FLAGS}")
	SET(CMAKE_CXX_FLAGS "/MP /Oi ${CMAKE_CXX_FLAGS}")

	set(Boost_USE_STATIC_LIBS OFF )
	set(Boost_USE_MULTITHREADED ON)
	set(Boost_USE_STATIC_RUNTIME OFF)
	
	set(LIBS ${LIBS} kernel32.lib)
	
	include_directories("C:\\usr\\vclibs\\include")
endif(MSVC)

if(MINGW OR MSYS OR UNIX)
	if(UNIX AND NOT APPLE)
		set(OpenGL_GL_PREFERENCE GLVND)
	elseif(MINGW OR MSYS)
		set(LIBS ${LIBS} -lmingw32)
		set(QT_DIR "c:/msys64/mingw64")
	endif(UNIX AND NOT APPLE)
	
	if(UNIX AND NOT APPLE)
		include_directories("/usr/include")
		include_directories("/usr/include/eigen3")
	elseif(MINGW OR MSYS)
		include_directories("c:/msys64/mingw64/include")
		include_directories("c:/msys64/mingw64/include/eigen3")
	endif(UNIX AND NOT APPLE)
endif(MINGW OR MSYS OR UNIX)

set(MAIN_SOURCE
	main.cpp
	main_window.hpp
	main_window.cpp
	gl_widget.hpp
	gl_widget.cpp
	key_handler.hpp
	key_handler.cpp
	common-cpp/fox/counter.hpp
	common-cpp/fox/counter.cpp
	common-cpp/fox/gfx/model_loader_obj.hpp
	common-cpp/fox/gfx/model_loader_obj.cpp
	common-cpp/fox/gfx/eigen_opengl.hpp
	common-cpp/fox/gfx/eigen_opengl.cpp
	common-cpp/fox/gfx/opengl_error_checker.h
	common-cpp/fox/gfx/opengl_error_checker.c
	shader.hpp
	shader_program.hpp
	uniform.hpp
	tinyobjloader/tiny_obj_loader.h
	tinyobjloader/tiny_obj_loader.cc
)

set(GLAD_SOURCE
	glad/glad.h
	KHR/khrplatform.h
	glad/glad.c
)

find_package(OpenGL REQUIRED)

find_package(Boost COMPONENTS program_options system filesystem REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

find_package(Qt5Core)
find_package(Qt5Widgets)
find_package(Qt5Gui)
find_package(Qt5OpenGL)

add_library(glad STATIC ${GLAD_SOURCE})
target_link_libraries(glad
	${OPENGL_LIBRARY}
	${LIBS}
)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
add_executable(${PROJECT_NAME} ${MAIN_SOURCE} ${COMMON_SOURCE} )
target_link_libraries(${PROJECT_NAME} ${OPENGL_LIBRARY} glad Qt5::Core Qt5::Widgets Qt5::Gui Qt5::OpenGL ${Boost_LIBRARIES})
set(CMAKE_AUTORCC OFF)
set(CMAKE_AUTOMOC OFF)

if(MSVC)
	set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
endif(MSVC)
