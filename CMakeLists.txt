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
	MK.cpp
	MK_io.cpp
	KID.cpp
	KEK.cpp
	DEK.cpp
	token.cpp
	AES256GCM.cpp
	CUI/main.cpp
	CUI/master.cpp
	CUI/util.cpp
)

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
