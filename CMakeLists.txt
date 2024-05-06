cmake_minimum_required(VERSION 3.21)

project(UntitledCLIParser VERSION 2.2.0.4)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 99)

include_directories(.)

option(WITH_UTF8 "Compiles with UTF-8 support. Requires utfcpp." ON)
if (WITH_UTF8 AND BUILD_VARIANT_VENDOR)
    add_subdirectory(ThirdParty/utfcpp)
endif()

set(UCLI_LIBRARY_TYPE "SHARED")
if (BUILD_VARIANT_STATIC)
    set(UCLI_LIBRARY_TYPE "STATIC")
endif()

set(UCLI_C_HEADERS "C/cucli.h")
set(UCLI_HEADERS "Common.h" "CLIParser.hpp")

add_library(UntitledCLIParser ${UCLI_LIBRARY_TYPE} C/cucli.cpp CLIParser.cpp ${UCLI_HEADERS} ${UCLI_C_HEADERS})

target_compile_definitions(UntitledCLIParser PRIVATE UVK_LOG_EXPORT_FROM_LIBRARY UVK_LIB_COMPILE)
target_compile_definitions(UntitledCLIParser PUBLIC UIMGUI_CLI_PARSER_MODULE_ENABLED)
list(APPEND compile_defs "UIMGUI_CLI_PARSER_MODULE_ENABLED")
if (WITH_UTF)
    target_compile_definitions(UntitledCLIParser PUBLIC UCLI_COMPILE_WITH_UTF_SUPPORT)
    list(APPEND compile_defs "UCLI_COMPILE_WITH_UTF_SUPPORT")
endif()

configure_file(UntitledCLIParser.pc.in UntitledCLIParser.pc @ONLY)

if (UIMGUI_INSTALL)
    install(TARGETS UntitledCLIParser
            LIBRARY DESTINATION lib64/
            ARCHIVE DESTINATION lib64/)
    install(FILES ${UCLI_HEADERS} DESTINATION include/UntitledCLIParser)
    install(FILES ${UCLI_C_HEADERS} DESTINATION include/UntitledCLIParser/C)
    install(FILES ${PROJECT_BINARY_DIR}/UntitledCLIParser.pc
            DESTINATION lib/pkgconfig/)
endif()
