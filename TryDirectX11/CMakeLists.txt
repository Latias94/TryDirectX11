cmake_minimum_required(VERSION 3.8)
set (CMAKE_CXX_STANDARD 17)

project (TryDirectX11)
option(WIN7_SYSTEM_SUPPORT "Windows7 users need to select this option!" OFF)

add_definitions(-D IS_DEBUG)

add_compile_definitions(UNICODE _UNICODE)
if (WIN7_SYSTEM_SUPPORT MATCHES ON)
    add_compile_definitions(_WIN32_WINNT=0x601)
endif ()

add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

# 查找所有编译好的 shader 二进制，并存在 shader_compile_files
file( GLOB shader_compile_files ${CMAKE_CURRENT_SOURCE_DIR}/*.cso )
# variable_watch(shader_compile_files) # debug用

# 复制所有编译好的 shader 二进制到执行文件夹
foreach( file_i ${shader_compile_files})
    file(COPY ${file_i} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
endforeach( file_i )

#if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/HLSL)
#    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/HLSL DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
#endif()

if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/Texture)
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Texture DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
endif()

if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/Model)
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Model DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
endif()

aux_source_directory(. DIR_SRCS)

add_executable(TryDirectX11 WIN32 ${DIR_SRCS})