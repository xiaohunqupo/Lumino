project(Example-Engine-EditorWithImGui)

find_package(imgui CONFIG REQUIRED)

#-------------------------------------------------------------------------------
# Targets
#-------------------------------------------------------------------------------
add_executable(${PROJECT_NAME} Main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE LuminoEngine imgui::imgui)
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "Examples")
