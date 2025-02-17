project(LuminoFFI)

#-------------------------------------------------------------------------------
# Files
#-------------------------------------------------------------------------------

file(GLOB LN_SOURCES_FFI include/LuminoFFI/*.h include/LuminoFFI/*.hpp src/*.cpp src/*.hpp)

source_group("FFI" FILES ${LN_SOURCES_FFI})

set(LN_SOURCES
    src/pch.hpp
    src/Internal.hpp
    ${LN_SOURCES_FFI}
)

#-------------------------------------------------------------------------------
# Configuration
#-------------------------------------------------------------------------------


#-------------------------------------------------------------------------------
# Targets
#-------------------------------------------------------------------------------

add_library(${PROJECT_NAME} SHARED ${LN_SOURCES})
add_library(lumino::${PROJECT_NAME} ALIAS ${PROJECT_NAME})
target_include_directories(
    ${PROJECT_NAME} INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_include_directories(${PROJECT_NAME} PRIVATE include src) # for PCH
target_link_libraries(${PROJECT_NAME} PRIVATE LuminoCore LuminoEngine)
target_link_libraries(${PROJECT_NAME} PUBLIC LuminoRuntime)

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "Lumino")
target_precompile_headers(${PROJECT_NAME} PRIVATE "src/pch.hpp")

#-------------------------------------------------------------------------------
# Installation
#-------------------------------------------------------------------------------
if (LUMINO_INSTALL)
    set(LN_TARGETS_NAME "${PROJECT_NAME}Targets")
    set(LN_CONFIG_NAME "${PROJECT_NAME}Config")

    install(
        TARGETS ${PROJECT_NAME}
        EXPORT LuminoExport
    )

    install(DIRECTORY include DESTINATION .)
endif()

#-------------------------------------------------------------------------------
# Sub-projects
#-------------------------------------------------------------------------------

if (LN_BUILD_TESTS)
    add_subdirectory(test)
endif()
