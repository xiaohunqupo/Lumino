﻿cmake_minimum_required (VERSION 3.0.0)
project(LuminoEngine-test)

#-------------------------------------------------------------------------------
# Files
#-------------------------------------------------------------------------------
file(GLOB LOCAL_SOURCES "*.hpp" "*.cpp")
source_group("Sources" FILES ${LOCAL_SOURCES})

#-------------------------------------------------------------------------------
# Targets
#-------------------------------------------------------------------------------
add_executable(${PROJECT_NAME} ${LOCAL_SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE .)
target_link_libraries(${PROJECT_NAME} LuminoEngine LuminoFFI GTest::gtest GTest::gtest_main)
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "Lumino")
target_precompile_headers(${PROJECT_NAME} PRIVATE "PCH.hpp")

add_test(NAME LuminoEngine-test-all COMMAND $<TARGET_FILE:LuminoEngine-test>)
