#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "glslangtests" for configuration "Debug"
set_property(TARGET glslangtests APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(glslangtests PROPERTIES
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/bin/glslangtests.exe"
  )

list(APPEND _IMPORT_CHECK_TARGETS glslangtests )
list(APPEND _IMPORT_CHECK_FILES_FOR_glslangtests "${_IMPORT_PREFIX}/bin/glslangtests.exe" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
