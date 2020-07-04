cmake_minimum_required(VERSION 3.6)
# cmake version 3.6 is required for Visual Studio startup project
project(shader-tester-qt)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

include_directories("common-cpp")

# used this to add common-cpp as a submodule
# git submodule add ssh://foxfire@foxfire256.ddns.net:27730/nas/dev/common-cpp common-cpp
# then manually add the following to .gitmodules
# url = git@github.com:foxfire256/common-cpp.git

# cmake .. -G "Visual Studio 16 2019" -A x64
if(MSVC)
	#set(CMAKE_PREFIX_PATH "C:\\vclibs\\vclibs")
	#set(CMAKE_INCLUDE_PATH "C:\\vclibs\\vclibs\\include")
	#set(CMAKE_LIBRARY_PATH "C:\\vclibs\\vclibs\\lib")
	ADD_DEFINITIONS(-D_SCL_SECURE_NO_WARNINGS)
	ADD_DEFINITIONS(-D_CRT_SECURE_NO_DEPRECATE)
	ADD_DEFINITIONS(-D_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS)
	
	SET(CMAKE_C_FLAGS "/MP ${CMAKE_C_FLAGS}")
	SET(CMAKE_CXX_FLAGS "/MP /std:c++17 /Oi ${CMAKE_CXX_FLAGS}")
	
	find_package(Qt5Core)
	find_package(Qt5Widgets)
	find_package(Qt5Gui)
	find_package(Qt5OpenGL)
	
	set(QT_LIBS Qt5::Core Qt5::Widgets Qt5::Gui Qt5::OpenGL)
	
	set(LIBS ${LIBS} kernel32.lib)
	
	set(RENDERER_LIBS ${RENDERER_LIBS} ${LIBS} opengl32.lib glu32.lib)
	
	include_directories("C:\\vclibs\\vclibs\\include")
endif(MSVC)

if(MINGW OR MSYS OR UNIX)
	if(UNIX AND NOT APPLE)
		set(CMAKE_PREFIX_PATH "/usr")
		set(CMAKE_INCLUDE_PATH "/usr/include")
		set(CMAKE_LIBRARY_PATH "/usr/lib")
	elseif(MINGW OR MSYS)
		set(CMAKE_PREFIX_PATH "c:/msys64/mingw64")
		set(CMAKE_INCLUDE_PATH "c:/msys64/mingw64/include")
		set(CMAKE_LIBRARY_PATH "c:/msys64/mingw64/lib")
	endif(UNIX AND NOT APPLE)
	
	if(UNIX AND NOT APPLE)
		set(LIBS ${LIBS})
		set(RENDERER_LIBS ${RENDERER_LIBS} -lOpenGL -lGLU)
	elseif(MINGW OR MSYS)
		set(LIBS ${LIBS} -lmingw32)
		set(RENDERER_LIBS ${RENDERER_LIBS} -lmingw32 -lopengl32 -lglu32)
	endif(UNIX AND NOT APPLE)
	
	if(UNIX AND NOT APPLE)
		# QT_DIR should already be defined
	elseif(MINGW OR MSYS)
		set(QT_DIR "c:/msys64/mingw64")
	endif(UNIX AND NOT APPLE)

	find_package(Qt5Core)
	find_package(Qt5Widgets)
	find_package(Qt5Gui)
	find_package(Qt5OpenGL)
	
	if(UNIX AND NOT APPLE)
		set(QT_LIBS Qt5::Core Qt5::Widgets Qt5::Gui Qt5::OpenGL)
		include_directories("/usr/include")
	elseif(MINGW OR MSYS)
		set(QT_LIBS Qt5::Core Qt5::Widgets Qt5::Gui Qt5::OpenGL)
		include_directories("c:/msys64/mingw64/include")
	endif(UNIX AND NOT APPLE)
	
	set(CMAKE_C_FLAGS_RELEASE "-Ofast")
	set(CMAKE_CXX_FLAGS_RELEASE "-Ofast")
	
	# WARNING: warnings are disabled
	set(CMAKE_C_FLAGS_DEBUG "-ggdb3 -Og")
	set(CMAKE_CXX_FLAGS_DEBUG "-ggdb3 -Og -w")

	# enable C11/C++17 for gcc and clang
	set(EXTRA_CXX_FLAGS "-std=c++17 -Wno-deprecated-declarations -pipe -frtti")
	set(EXTRA_C_FLAGS "-std=c17 -Wno-deprecated-declarations -pipe")
	
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_C_FLAGS}")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${EXTRA_CXX_FLAGS}")
	
	# pipe keeps compilation in memory
	set(CMAKE_LD_FLAGS "-pipe")
endif(MINGW OR MSYS OR UNIX)

set(MAIN_SOURCE
	main.cpp
	main_window.hpp
	main_window.cpp
	gl_widget.hpp
	gl_widget.cpp
	common-cpp/fox/counter.hpp
	common-cpp/fox/counter.cpp
	common-cpp/fox/gfx/model_loader_obj.hpp
	common-cpp/fox/gfx/model_loader_obj.cpp
)

# static libraries don't work so well on Apple for some reason
set(LIBRARY_TYPE STATIC)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
add_executable(${PROJECT_NAME} ${MAIN_SOURCE} ${COMMON_SOURCE} )
target_link_libraries(${PROJECT_NAME} ${QT_LIBS} ${RENDERER_LIBS})
set(CMAKE_AUTORCC OFF)
set(CMAKE_AUTOMOC OFF)

if(MSVC)
	set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
endif(MSVC)

MESSAGE( STATUS "MINGW: " ${MINGW} )
MESSAGE( STATUS "MSYS: " ${MSYS} )
MESSAGE( STATUS "MSVC: " ${MSVC} )
MESSAGE( STATUS "APPLE: " ${APPLE} )
get_target_property(QtCore_location Qt5::Core LOCATION)
MESSAGE( STATUS "QtCore_location: " ${QtCore_location})
get_target_property(QtWdigets_location Qt5::Widgets LOCATION)
MESSAGE( STATUS "QtWdigets_location: " ${QtWdigets_location})
