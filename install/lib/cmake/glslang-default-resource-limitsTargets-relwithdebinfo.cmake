#----------------------------------------------------------------
# Generated CMake target import file for configuration "RelWithDebInfo".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "glslang-default-resource-limits" for configuration "RelWithDebInfo"
set_property(TARGET glslang-default-resource-limits APPEND PROPERTY IMPORTED_CONFIGURATIONS RELWITHDEBINFO)
set_target_properties(glslang-default-resource-limits PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELWITHDEBINFO "CXX"
  IMPORTED_LOCATION_RELWITHDEBINFO "${_IMPORT_PREFIX}/lib/glslang-default-resource-limits.lib"
  )

list(APPEND _IMPORT_CHECK_TARGETS glslang-default-resource-limits )
list(APPEND _IMPORT_CHECK_FILES_FOR_glslang-default-resource-limits "${_IMPORT_PREFIX}/lib/glslang-default-resource-limits.lib" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
