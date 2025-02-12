cmake_minimum_required(VERSION 3.20)
# UTF-8 source
add_compile_options("$<$<AND:$<C_COMPILER_ID:MSVC>,$<COMPILE_LANGUAGE:C>>:/utf-8>")
add_compile_options("$<$<AND:$<CXX_COMPILER_ID:MSVC>,$<COMPILE_LANGUAGE:CXX>>:/utf-8>")
add_compile_options("$<$<AND:$<CXX_COMPILER_ID:MSVC>,$<COMPILE_LANGUAGE:CXX>>:/Zc:__cplusplus>")

set(PROJECT_NAME krkrz)
set(PROJECT_VERSION 2.0.0)

project(${PROJECT_NAME} VERSION ${PROJECT_VERSION})

option(USENEON "use neon" OFF)
option(USEOPENGL "use opengl" OFF)
option(USESJIS "use sjis" OFF)
option(BUILDLIB "build library" OFF)

if(WIN32 AND (CMAKE_SIZEOF_VOID_P EQUAL 8))
set(WIN64 1)
endif()

if(WIN32)
if(WIN64)
set(EXENAME ${PROJECT_NAME}64)
set(LIBNAME lib${PROJECT_NAME})
set(PLUGIN_DIR "plugin64")
else()
set(EXENAME ${PROJECT_NAME})
set(LIBNAME lib${PROJECT_NAME}32)
set(PLUGIN_DIR "plugin")
endif()
else()
set(EXENAME ${PROJECT_NAME})
set(LIBNAME lib${PROJECT_NAME})
set(PLUGIN_DIR "plugin")
endif()

if (WIN32)
set(CMAKE_RC_FLAGS "")
# nasm setup
set( CMAKE_ASM_NASM_FLAGS "-O1 -safeseh -Xvc -fwin32 -DWIN32 -I${CMAKE_CURRENT_SOURCE_DIR} -I${CMAKE_CURRENT_SOURCE_DIR}/visual/IA32/" )
set( CMAKE_ASM_NASM_SOURCE_FILE_EXTENSIONS "asm;nasm;nas")
find_program(CMAKE_ASM_NASM_COMPILER NAMES nasm)
enable_language(ASM_NASM)
endif()

set(KRKRZ_DEFINES
TJS_JP_LOCALIZED
TVP_ENABLE_EXECUTE_AT_EXCEPTION
TVP_LOG_TO_COMMANDLINE_CONSOLE
UNICODE
_UNICODE
# ファイル名をノーマライズしない
TVP_NO_NORMALIZE_PATH
# ファイル検索時はケース無視
TVP_AUTOPATH_IGNORECASE
# ローカルパス参照のcaseを強制参照
TVP_LOCALFILE_FORCE_CASESENSITIVE
__CODEGUARD__
TVP_USE_TURBO_JPEG_API
)

if (USESJIS)
list(APPEND KRKRZ_DEFINES
# 標準をSJISにするかどうか
TVP_TEXT_READ_ANSI_MBCS
)
endif()

if (WIN32)
list(APPEND KRKRZ_DEFINES
TJS_TEXT_OUT_CRLF
)
endif()

include(sources.cmake)

if (EXISTS "win32/custom.cmake")
include(win32/custom.cmake)
endif()

# libraries

find_package(ZLIB REQUIRED)
find_package(PNG MODULE)
find_package(freetype CONFIG REQUIRED)
find_package(JPEG REQUIRED)
find_package(Ogg CONFIG REQUIRED)
find_package(Opus REQUIRED)
find_package(oniguruma CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(Vorbis CONFIG REQUIRED)

find_package(libjpeg-turbo CONFIG REQUIRED)

find_library(OPUSFILE NAMES opusfile)

find_path(PICOJSON_INCLUDE_DIRS "picojson/picojson.h")

find_package(Threads REQUIRED)


if (BUILDLIB)

# ----------------------------------------------------
# krkrz generic library
# ----------------------------------------------------

message("krkrz generic library")

add_library( ${LIBNAME} EXCLUDE_FROM_ALL STATIC
	${KRKRZ_SRC} 
	${KRKRZ_SRC_OPENGL} 
	${KRKRZ_SRC_GENERIC} 
)

set_target_properties(${LIBNAME} PROPERTIES 
    PUBLIC_HEADER "${KRKRZ_PUBLIC_HEADER}"
#   DEBUG_POSTFIX "_d"
)

target_include_directories( ${LIBNAME} PUBLIC
	${KRKRZ_INC}
	${KRKRZ_INC_GENERIC}
	${KRKRZ_INCLUDE_PLATFORM}
)

target_include_directories( ${LIBNAME} PRIVATE 
	${JPEG_INCLUDE_DIR}
	${PICOJSON_INCLUDE_DIRS}
)

target_compile_definitions( ${LIBNAME} PUBLIC
	${KRKRZ_DEFINES}
	__GENERIC__
	TVP_DONT_AUTOLOAD_PROVINCE
	TVP_DONT_AUTOLOAD_MASK
	${KRKRZ_DEFINES_PLATFORM}  
	${KRKRZ_DEFINES_OPENGL}
)

target_link_libraries( ${LIBNAME} PUBLIC
	freetype
	${JPEG_LIBRARIES}
	$<IF:$<TARGET_EXISTS:libjpeg-turbo::turbojpeg>,libjpeg-turbo::turbojpeg,libjpeg-turbo::turbojpeg-static>
	${OPUSFILE}
	Ogg::ogg
	Opus::opus
	Vorbis::vorbis
	Vorbis::vorbisenc 
	Vorbis::vorbisfile
	ZLIB::ZLIB
	oniguruma::onig
	PNG::PNG
	glm::glm
	Threads::Threads
)

else() # BUILDLIB

# ----------------------------------------------------
# krkrz original win32 version
# ----------------------------------------------------

message("krkrz win32")

# krmovie
add_subdirectory( win32/movie )
set_target_properties(krmovie PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# krkrz executable

add_executable( ${EXENAME} WIN32
	${KRKRZ_SRC} 
	${KRKRZ_SRC_OPENGL} 
	${KRKRZ_SRC_WIN32} 
	${KRKRZ_SRC_WIN32_SSE} 
	${KRKRZ_SRC_WIN32_OPENGL} 
)

target_include_directories( ${EXENAME} PRIVATE 
	${KRKRZ_INC}
	${KRKRZ_INC_WIN32} 
	${JPEG_INCLUDE_DIR}
	${PICOJSON_INCLUDE_DIRS}
)

target_compile_definitions( ${EXENAME} PRIVATE
	${KRKRZ_DEFINES}
	NO_STRICT
	__WIN32__
	TJS_DEBUG_DUMP_STRING
	TVP_REPORT_HW_EXCEPTION
	${KRKRZ_DEFINES_PLATFORM}  
	${KRKRZ_DEFINES_OPENGL}
)

target_link_libraries( ${EXENAME} PUBLIC
	freetype
	${JPEG_LIBRARIES}
	$<IF:$<TARGET_EXISTS:libjpeg-turbo::turbojpeg>,libjpeg-turbo::turbojpeg,libjpeg-turbo::turbojpeg-static>
	${OPUSFILE}
	Ogg::ogg
	Opus::opus
	Vorbis::vorbis
	Vorbis::vorbisenc 
	Vorbis::vorbisfile
	ZLIB::ZLIB
	oniguruma::onig
	PNG::PNG
	glm::glm
)

install(TARGETS ${EXENAME}
	DESTINATION .
)

install(TARGETS krmovie
	LIBRARY DESTINATION ${PLUGIN_DIR}
	RUNTIME DESTINATION ${PLUGIN_DIR}
)

endif() # BUILDLIB
