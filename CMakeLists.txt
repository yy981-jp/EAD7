# set(CMAKE_BUILD_TYPE Debug)
# set(CMAKE_BUILD_TYPE Releases)



cmake_minimum_required(VERSION 3.20...4.1)
project(EAD7) # Encryption and decryption ver7
add_compile_options(-fdiagnostics-color=always)

set(CMAKE_CXX_STANDARD 23)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)
find_package(Boost REQUIRED COMPONENTS locale)

# MOC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)


file(GLOB_RECURSE SOURCES src/*.cpp src/*.ui)

if(CMAKE_BUILD_TYPE STREQUAL "Releases")
	add_executable(${PROJECT_NAME} WIN32 ${SOURCES})

	file(GLOB LIBSRC src/*.cpp)
	list(REMOVE_ITEM LIBSRC "${CMAKE_SOURCE_DIR}/src/main.cpp")
	add_library(LIBEXE STATIC ${LIBSRC})
	set_target_properties(LIBEXE PROPERTIES OUTPUT_NAME "EAD7")
else()
	add_executable(${PROJECT_NAME} ${SOURCES})
endif()


target_include_directories(${PROJECT_NAME} PRIVATE 
	"C:/msys64/mingw64/include/qt6"
	"${CMAKE_SOURCE_DIR}/src/GUI/widgets"
)

# ライブラリ
target_link_libraries(${PROJECT_NAME} PRIVATE
	cryptopp
	sodium
	Boost::locale
	Qt6Core
	Qt6Gui
	Qt6Widgets
)
