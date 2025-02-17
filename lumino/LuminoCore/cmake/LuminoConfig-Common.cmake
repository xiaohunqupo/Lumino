
#-------------------------------------------------------------------------------
# Configuration
#-------------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 17)
set(LN_LIB_OUTPUT_LOCAL_DIR "lib/") # TODO: deprecated?

# Unicode char set.
add_definitions(-DUNICODE)
add_definitions(-D_UNICODE)

set(LN_LINK_CMAKE_SHARED_LIB_PACKAGE OFF)
if (MSVC)
    if (NOT LN_MSVC_STATIC_RUNTIME)
        set(LN_LINK_CMAKE_SHARED_LIB_PACKAGE ON)
    endif()
endif()


#-------------------------------------------------------------------------------
# functions
#-------------------------------------------------------------------------------

macro(ln_use_msvc_static_runtime_library)
    if (MSVC)
        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

        foreach (flag CMAKE_C_FLAGS_DEBUG_INIT CMAKE_C_FLAGS_MINSIZEREL_INIT CMAKE_C_FLAGS_RELEASE_INIT CMAKE_C_FLAGS_RELWITHDEBINFO_INIT CMAKE_CXX_FLAGS_DEBUG_INIT CMAKE_CXX_FLAGS_MINSIZEREL_INIT CMAKE_CXX_FLAGS_RELEASE_INIT CMAKE_CXX_FLAGS_RELWITHDEBINFO_INIT)
            if (${flag} MATCHES "/MD")
                string(REGEX REPLACE "/MD" "/MT" ${flag} "${${flag}}")
            endif()
            if (${flag} MATCHES "/MDd")
                string(REGEX REPLACE "/MDd" "/MTd" ${flag} "${${flag}}")
            endif()
        endforeach()
        foreach (flag CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE)
            if (${flag} MATCHES "/MD")
                string(REGEX REPLACE "/MD" "/MT" ${flag} "${${flag}}")
            endif()
            if (${flag} MATCHES "/MDd")
                string(REGEX REPLACE "/MDd" "/MTd" ${flag} "${${flag}}")
            endif()
        endforeach()
    endif()
endmacro()

function(ln_assign_source_group ROOT_DIR FILES)
    foreach(_source IN ITEMS ${FILES})
        if (IS_ABSOLUTE "${_source}")
            file(RELATIVE_PATH _source_rel "${ROOT_DIR}" "${_source}")
        else()
            set(_source_rel "${_source}")
        endif()
        get_filename_component(_source_path "${_source_rel}" PATH)
        string(REPLACE "/" "\\" _source_path_msvc "${_source_path}")
        source_group("${_source_path_msvc}" FILES "${_source}")
    endforeach()
endfunction()
