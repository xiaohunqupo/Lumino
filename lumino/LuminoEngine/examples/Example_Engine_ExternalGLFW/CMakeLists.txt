project(Example_Engine_ExternalGLFW)

#-------------------------------------------------------------------------------
# Targets
#-------------------------------------------------------------------------------
add_executable(${PROJECT_NAME} Main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE LuminoEngine glfw glad::glad)
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "Examples")
