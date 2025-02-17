
cmake_minimum_required(VERSION 3.0)
project(LuminoRuntime)
include(cmake/LuminoConfig-LuminoRuntime.cmake)

#-------------------------------------------------------------------------------
# Files
#-------------------------------------------------------------------------------
set(LN_SOURCES_Common
    include/LuminoEngine/RuntimeModule.hpp
    src/pch.hpp
    src/Internal.hpp
    src/RegisterTypes.cpp
    src/RuntimeModule.cpp
)
file(GLOB LN_SOURCES_Runtime include/LuminoEngine/Runtime/*.hpp include/LuminoEngine/Runtime/detail/*.hpp src/Runtime/*.cpp src/Runtime/*.hpp)
file(GLOB LN_SOURCES_Base include/LuminoEngine/Base/*.hpp src/Base/*.cpp src/Base/*.hpp)
file(GLOB LN_SOURCES_Reflection include/LuminoEngine/Reflection/*.hpp src/Reflection/*.cpp src/Reflection/*.hpp)
file(GLOB LN_SOURCES_Engine include/LuminoEngine/Engine/*.hpp src/Engine/*.cpp src/Engine/*.hpp)
file(GLOB LN_SOURCES_Asset include/LuminoEngine/Asset/*.hpp src/Asset/*.cpp src/Asset/*.hpp)
file(GLOB LN_SOURCES_Graphics include/LuminoEngine/Graphics/*.hpp src/Graphics/*.cpp src/Graphics/*.hpp)

# Grouping.
source_group("" FILES ${LN_SOURCES_Common})
source_group("Runtime" FILES ${LN_SOURCES_Runtime})
source_group("Base" FILES ${LN_SOURCES_Base})
source_group("Reflection" FILES ${LN_SOURCES_Reflection})
source_group("Engine" FILES ${LN_SOURCES_Engine})
source_group("Asset" FILES ${LN_SOURCES_Asset})
source_group("Graphics" FILES ${LN_SOURCES_Graphics})

set(LN_SOURCES
    ${LN_SOURCES_Common}
    ${LN_SOURCES_Runtime}
    ${LN_SOURCES_Base}
    ${LN_SOURCES_Reflection}
    ${LN_SOURCES_Engine}
    ${LN_SOURCES_Asset}
    ${LN_SOURCES_Graphics}
)

#-------------------------------------------------------------------------------
# Configuration
#-------------------------------------------------------------------------------
set(LINK_LIBS
    LuminoCore
    yaml-cpp
)

#if (NOT DEFINED EMSCRIPTEN)
#    list(APPEND LINK_LIBS CURL::libcurl)
#endif()

#-------------------------------------------------------------------------------
# Targets
#-------------------------------------------------------------------------------
add_library(${PROJECT_NAME} STATIC ${LN_SOURCES})
add_library(lumino::${PROJECT_NAME} ALIAS ${PROJECT_NAME})
target_include_directories(
    ${PROJECT_NAME} INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_include_directories(${PROJECT_NAME} PRIVATE include src)
target_link_libraries(${PROJECT_NAME} PRIVATE ${LINK_LIBS})

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "Lumino")
target_precompile_headers(${PROJECT_NAME} PRIVATE "src/pch.hpp")

#-------------------------------------------------------------------------------
# Installation
#-------------------------------------------------------------------------------
if (LUMINO_INSTALL)
    install(
        TARGETS     ${PROJECT_NAME}
        EXPORT      LuminoExport
    )

    install(DIRECTORY include DESTINATION .)

    # install(EXPORT) で出力される LuminoCoreConfig.cmake は、同一フォルダの
    # LuminoCoreConfig-*.cmake を include しようとするため、それにマッチするファイル名にする。
    install(FILES cmake/LuminoConfig-${PROJECT_NAME}.cmake DESTINATION ${LN_EXPORT_DESTINATION_DIR})
endif()

#-------------------------------------------------------------------------------
# Sub-projects
#-------------------------------------------------------------------------------
if (LN_BUILD_TESTS)
    add_subdirectory(test)
endif()

