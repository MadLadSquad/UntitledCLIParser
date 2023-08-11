cmake_minimum_required(VERSION 3.21)

project(UntitledCLIParser)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 99)

include_directories(.)

option(WITH_UTF8 "Compiles with UTF-8 support. Requires utfcpp." ON)

set(UCLI_LIBRARY_TYPE "SHARED")
if (BUILD_VARIANT_STATIC)
    set(UCLI_LIBRARY_TYPE "STATIC")
endif()

set(UCLI_HEADERS "Common.h" "CLIParser.hpp" "C/cucli.h")

add_library(UntitledCLIParser ${UCLI_LIBRARY_TYPE} C/cucli.cpp CLIParser.cpp ${UCLI_HEADERS})

target_compile_definitions(UntitledCLIParser PUBLIC UIMGUI_CLI_PARSER_MODULE_ENABLED)
if (WITH_UTF)
    target_compile_definitions(UntitledCLIParser PUBLIC UCLI_COMPILE_WITH_UTF_SUPPORT)
endif()

configure_file(UntitledCLIParser.pc.in UntitledCLIParser.pc @ONLY)

if (UIMGUI_INSTALL)
    install(TARGETS UntitledCLIParser
            LIBRARY DESTINATION lib/
            ARCHIVE DESTINATION lib/)
    install(FILES ${UCLI_HEADERS} DESTINATION include/UntitledCLIParser)
    install(FILES ${PROJECT_BINARY_DIR}/UntitledCLIParser.pc
            DESTINATION lib/pkgconfig/)
endif()
