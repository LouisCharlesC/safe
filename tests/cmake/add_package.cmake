macro(add_package name depname git_repo git_rev mod_dir)
    unset(${name}_FOUND CACHE) # needed for correct behaviour on rebuilds

    if (DEPS STREQUAL "AUTO")
        find_package (${name} CONFIG QUIET)
    elseif (DEPS STREQUAL "LOCAL")
        find_package (${name} REQUIRED CONFIG)
    endif()

    if (NOT ${name}_FOUND)
        include(FetchContent)
        FetchContent_Declare (${depname}
            GIT_REPOSITORY ${git_repo}
            GIT_TAG ${git_rev})
        FetchContent_MakeAvailable (${depname})
        FetchContent_GetProperties(${depname})
        if(NOT ${depname}_POPULATED)
            FetchContent_Populate(${depname})
            add_subdirectory(${${depname}_SOURCE_DIR} ${${depname}_BINARY_DIR} EXCLUDE_FROM_ALL) # prevent installing of dependencies
        endif()
        set (${name}_POPULATED TRUE) # this is supposed to be done in MakeAvailable but it seems not to?!?
        list(APPEND CMAKE_MODULE_PATH "${${depname}_SOURCE_DIR}/${mod_dir}")
    endif()
endmacro()
