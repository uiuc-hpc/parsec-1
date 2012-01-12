#
# Internal module for DAGuE. 
# Setup the minimal environment to compile and generate .JDF files. 
#

#
# This macro creates a rule for every jdf basename passed in SOURCES.
# The OUTPUTLIST contains the list of files generated by the maxro.
#
macro(jdf_rules jdf_rules_OUTPUTLIST jdf_rules_SOURCES)
  foreach(jdf_rules_SOURCE ${jdf_rules_SOURCES})
    # Remove .jdf if present
    string(REGEX REPLACE ".jdf" "" jdf_rules_SRC ${jdf_rules_SOURCE}) 
    string(REGEX REPLACE "^(.*/)*(.+)\\.*.*" "\\2" jdf_rules_BSRC ${jdf_rules_SRC})
    set(jdf_rules_OSRC "${jdf_rules_BSRC}")
    GET_PROPERTY(ADDITIONAL_DAGUEPP_CFLAGS SOURCE ${jdf_rules_SOURCE} PROPERTY ADDITIONAL_DAGUEPP_CFLAGS)
 
    get_source_file_property(jdf_rules_IsInBinaryDir ${jdf_rules_SOURCE} IS_IN_BINARY_DIR )

    # If the file is generated in a different binary dir, 
    # we force the dependency on the generated file
    # otherwise we let cmake choose the correct file, it is so good for that.
    if( jdf_rules_IsInBinaryDir )

      add_custom_command(
        OUTPUT ${jdf_rules_OSRC}.h ${jdf_rules_OSRC}.c
        COMMAND ${daguepp_EXE} ${DAGUEPP_CFLAGS} ${ADDITIONAL_DAGUEPP_CFLAGS} -i ${jdf_rules_SRC}.jdf -o ${jdf_rules_OSRC} -f ${jdf_rules_BSRC}
        MAIN_DEPENDENCY ${CMAKE_CURRENT_BINARY_DIR}/${jdf_rules_SRC}.jdf daguepp
        DEPEND ${CMAKE_CURRENT_BINARY_DIR}/${jdf_rules_SRC}.jdf)

    else( jdf_rules_IsInBinaryDir )

      add_custom_command(
        OUTPUT ${jdf_rules_OSRC}.h ${jdf_rules_OSRC}.c
        COMMAND ${daguepp_EXE} ${DAGUEPP_CFLAGS} ${ADDITIONAL_DAGUEPP_CFLAGS} -i ${jdf_rules_SRC}.jdf -o ${jdf_rules_OSRC} -f ${jdf_rules_BSRC}
        MAIN_DEPENDENCY ${jdf_rules_SRC}.jdf daguepp
        DEPEND ${jdf_rules_SRC}.jdf)

    endif( jdf_rules_IsInBinaryDir )

    list(APPEND ${jdf_rules_OUTPUTLIST} "${CMAKE_CURRENT_BINARY_DIR}/${jdf_rules_OSRC}.h;${CMAKE_CURRENT_BINARY_DIR}/${jdf_rules_OSRC}.c")

  endforeach()
  #
  # Mark all generated files as such.
  #
  set_source_files_properties(${jdf_rules_OUTPUTLIST} PROPERTIES GENERATED 1)
endmacro(jdf_rules)

