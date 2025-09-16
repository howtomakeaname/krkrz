cmake_minimum_required(VERSION 3.20)
# UTF-8 source
add_compile_options("$<$<AND:$<C_COMPILER_ID:MSVC>,$<COMPILE_LANGUAGE:C>>:/utf-8>")
add_compile_options("$<$<AND:$<CXX_COMPILER_ID:MSVC>,$<COMPILE_LANGUAGE:CXX>>:/utf-8>")
add_compile_options("$<$<AND:$<CXX_COMPILER_ID:MSVC>,$<COMPILE_LANGUAGE:CXX>>:/Zc:__cplusplus>")

set(PROJECT_NAME krkrz)
set(PROJECT_VERSION 2.0.0)

project(${PROJECT_NAME} VERSION ${PROJECT_VERSION})

option(USE_NEON "use neon" OFF)
option(USE_SJIS "use sjis" OFF)
option(USE_OPENGL "use opengl" ON)

option(BUILD_LIB "build library version" OFF)

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
)

if (USE_SJIS)
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
find_package(Ogg CONFIG REQUIRED)
find_package(Opus)
find_package(oniguruma CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(Vorbis CONFIG REQUIRED)

find_package(libjpeg-turbo CONFIG)

# libjpeg-turbo::turbojpeg が見つからない場合
if (NOT libjpeg-turbo_FOUND)
	message("libjpeg-turbo not found, trying to find libjpeg")
	find_package(JPEG QUIET)
else()
	message("libjpeg-turbo found")
	set(JPEG_INCLUDE_DIR ${libjpeg-turbo_INCLUDE_DIRS})
	set(JPEG_LIBRARIES $<IF:$<TARGET_EXISTS:libjpeg-turbo::turbojpeg>,libjpeg-turbo::turbojpeg,libjpeg-turbo::turbojpeg-static>)
	set(JPEG_TURBO_FOUND TRUE)
	list(APPEND KRKRZ_DEFINES TVP_USE_TURBO_JPEG_API)
endif()

find_package(opusfile CONFIG)

if (0)
find_library(OPUSFILE NAMES opusfile)
if(OPUSFILE_FOUND AND NOT TARGET opusfile)
  find_path(OPUSFILE_INCLUDE_DIRS "opus/opusfile.h")
  add_library(opusfile UNKNOWN IMPORTED)
  set_target_properties(opusfile PROPERTIES
    IMPORTED_LINK_INTERFACE_LANGUAGES "C"
    IMPORTED_LOCATION "${OPUSFILE_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${OPUSFILE_INCLUDE_DIR}"
    )
endif()
endif()

find_path(PICOJSON_INCLUDE_DIRS "picojson/picojson.h")

find_package(Threads REQUIRED)

find_package(plog CONFIG)

find_package(fmt CONFIG REQUIRED)

if (BUILD_LIB)

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
	Ogg::ogg
	Vorbis::vorbis
	Vorbis::vorbisenc 
	Vorbis::vorbisfile
	ZLIB::ZLIB
	oniguruma::onig
	fmt::fmt
	PNG::PNG
	glm::glm
	Threads::Threads
	$<IF:$<TARGET_EXISTS:Opus::opus>,Opus::opus,>
	$<IF:$<TARGET_EXISTS:OpusFile::opusfile>,OpusFile::opusfile,>
	$<IF:$<TARGET_EXISTS:opusfile>,opusfile,>
	$<IF:$<TARGET_EXISTS:plog::plog>,plog::plog,>
)

# リソースデータ一式
set(RESOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/resource)
file(GLOB RESOURCE_FILES "${RESOURCE_DIR}/*")

if (WIN32)

# `.rc` ファイルを生成
set(RC_FILE ${CMAKE_BINARY_DIR}/resources.rc)
file(WRITE ${RC_FILE} "")

foreach(RESOURCE ${RESOURCE_FILES})
    get_filename_component(NAME ${RESOURCE} NAME)
    file(APPEND ${RC_FILE} "${NAME} BINARY \"${RESOURCE}\"\n")
endforeach()

elseif(ANDROID)

# なにもしない

else()

# objcopy のコマンドを探す
find_program(OBJCOPY objcopy)

# 各バイナリをオブジェクトファイルに変換
foreach(RESOURCE ${RESOURCE_FILES})
    get_filename_component(NAME ${RESOURCE} NAME)
    set(OBJ_FILE ${CMAKE_BINARY_DIR}/resource_obj/${NAME}.o)

    add_custom_command(
        OUTPUT ${OBJ_FILE}
        COMMAND ${OBJCOPY} -I binary -O elf64-x86-64 -B i386 ${RESOURCE} ${OBJ_FILE}
        DEPENDS ${RESOURCE}
    )

    list(APPEND RESOURCE_OBJECTS ${OBJ_FILE})
endforeach()

set(RESOURCE_LIB resources)

# リソースを含むライブラリを作成
add_library(${RESOURCE_LIB} STATIC ${RESOURCE_OBJECTS})
set_property(TARGET ${RESOURCE_LIB} PROPERTY LINKER_LANGUAGE C)

endif()


else() # !BUILD_LIB

# ----------------------------------------------------
# krkrz original win32 version
# ----------------------------------------------------

if (WIN32)
message("krkrz original based win32 executable")
else()
message(FATAL_ERROR "Unsupported platform")
endif()

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

target_link_libraries( ${EXENAME} PRIVATE
	freetype
	${JPEG_LIBRARIES}
	OpusFile::opusfile
	Ogg::ogg
	Opus::opus
	Vorbis::vorbis
	Vorbis::vorbisenc 
	Vorbis::vorbisfile
	ZLIB::ZLIB
	oniguruma::onig
	fmt::fmt
	PNG::PNG
	glm::glm
 	plog::plog
)

install(TARGETS ${EXENAME}
	DESTINATION .
)

install(TARGETS krmovie
	LIBRARY DESTINATION ${PLUGIN_DIR}
	RUNTIME DESTINATION ${PLUGIN_DIR}
)

endif() # !BUILD_LIB
