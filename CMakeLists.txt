set(CMAKE_BUILD_TYPE Debug)
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



set(SOURCES
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
	del.cpp
	CUI/main.cpp
	CUI/admin.cpp
	CUI/util.cpp
	GUI/main.cpp
	GUI/util.cpp
	GUI/main.ui
	GUI/widgets/fileButton.cpp
)

add_executable(${PROJECT_NAME} WIN32 ${SOURCES} ${HEADERS})
target_include_directories(${PROJECT_NAME} PRIVATE 
	"C:/msys64/mingw64/include/qt6"
	"${CMAKE_SOURCE_DIR}/GUI/widgets"
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
