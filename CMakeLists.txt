cmake_minimum_required(VERSION 3.16)
project(EAD7) # Encryption and decryption ver7
add_compile_options(-fdiagnostics-color=always)

set(CMAKE_CXX_STANDARD 23)

# ソースファイル
set(SOURCES
	# test.cpp
	main.cpp
	base.cpp
	util.cpp
	master.cpp
	MK.cpp
	io.cpp
	KID.cpp
)

add_compile_options(-fexec-charset=cp932)
# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# ライブラリをリンク
target_link_libraries(${PROJECT_NAME} PRIVATE
	cryptopp
	sodium
)
