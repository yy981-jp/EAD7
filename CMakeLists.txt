set(CMAKE_BUILD_TYPE Debug)



cmake_minimum_required(VERSION 3.16)
project(EAD7) # Encryption and decryption ver7
add_compile_options(-fdiagnostics-color=always)

set(CMAKE_CXX_STANDARD 23)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets OpenGLWidgets)

# MOC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(SOURCES
	# test.cpp
	main.ui
	main.cpp
	base.cpp
	def.cpp
	util.cpp
	master.cpp
	MK.cpp
	io.cpp
	KID.cpp
	HKDF.cpp
	token.cpp
)

# add_compile_options(-fexec-charset=cp932)
# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})
target_include_directories(${PROJECT_NAME} PRIVATE "C:/msys64/mingw64/include/qt6")

# ライブラリ
target_link_libraries(${PROJECT_NAME}
	cryptopp
	sodium
	# Qt6Core
	# Qt6Gui
	# Qt6Widgets
	# Qt6OpenGLWidgets
)

# target_link_options(${PROJECT_NAME} PRIVATE -mconsole)