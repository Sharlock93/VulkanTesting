#----------------------------------------------------------------
# Generated CMake target import file for configuration "RelWithDebInfo".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "SPVRemapper" for configuration "RelWithDebInfo"
set_property(TARGET SPVRemapper APPEND PROPERTY IMPORTED_CONFIGURATIONS RELWITHDEBINFO)
set_target_properties(SPVRemapper PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELWITHDEBINFO "CXX"
  IMPORTED_LOCATION_RELWITHDEBINFO "${_IMPORT_PREFIX}/lib/SPVRemapper.lib"
  )

list(APPEND _IMPORT_CHECK_TARGETS SPVRemapper )
list(APPEND _IMPORT_CHECK_FILES_FOR_SPVRemapper "${_IMPORT_PREFIX}/lib/SPVRemapper.lib" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
