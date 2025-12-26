# Findgflags.cmake
find_package(PkgConfig REQUIRED)

pkg_check_modules(gflags REQUIRED gflags)

if(NOT TARGET gflags::gflags)
    add_library(gflags::gflags INTERFACE IMPORTED)
    target_include_directories(gflags::gflags INTERFACE ${gflags_INCLUDE_DIR})
    set_target_properties(gflags::gflags PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${gflags_INCLUDE_DIRS}"
        INTERFACE_COMPILE_OPTIONS "${gflags_CFLAGS_OTHER}"
        INTERFACE_LINK_LIBRARIES "${gflags_LIBRARIES}"
        INTERFACE_LINK_DIRECTORIES "${gflags_LIBRARY_DIRS}"
    )
endif()

message(STATUS "GIANLUCA ${gflags_LIBRARIES}")