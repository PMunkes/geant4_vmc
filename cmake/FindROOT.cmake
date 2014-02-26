#------------------------------------------------
# The Virtual Monte Carlo examples
# Copyright (C) 2014 Ivana Hrivnacova
# All rights reserved.
#
# For the licensing terms see geant4_vmc/LICENSE.
# Contact: vmc@pcroot.cern.ch
#-------------------------------------------------

# Adopted from Virtual Geometry Model and ROOT (ROOT_GENERATE_DICTIONARY)
# (http://ivana.home.cern.ch/ivana/VGM.html)
# I. Hrivnacova, 31/01/2014
#
# The CMake build for Virtual Geometry Model is a result of a merge 
# of the CMake configuration files kindly provided 
# by Florian Uhlig, GSI and Pere Mato, CERN.
# (http://ivana.home.cern.ch/ivana/VGM.html)

# - Try to find ROOT instalation
# This module sets up ROOT information 
# It defines:
# ROOT_FOUND          If ROOT is found
# ROOT_INCLUDE_DIR    PATH to the include directory
# ROOT_LIBRARIES      Most common libraries
# ROOT_LIBRARY_DIR    PATH to the library directory 
# ROOT_ARCH           ROOT build architecture

#message(STATUS "Looking for ROOT ...")

set(ROOT_FOUND FALSE)

# First search for root-config executable on system path
# or path defined via user setting (ROOT_DIR)
# or path defined via ROOTSYS environment variable

find_program(ROOT_CONFIG_EXECUTABLE root-config PATHS
  $ROOT_DIR/bin
  $ENV{ROOTSYS}/bin
  )

if(ROOT_CONFIG_EXECUTABLE)
  set(ROOT_FOUND TRUE)

  execute_process(
    COMMAND ${ROOT_CONFIG_EXECUTABLE} --prefix 
    OUTPUT_VARIABLE ROOT_PREFIX 
    OUTPUT_STRIP_TRAILING_WHITESPACE)

  execute_process(
    COMMAND ${ROOT_CONFIG_EXECUTABLE} --version 
    OUTPUT_VARIABLE ROOT_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE)

  execute_process(
    COMMAND ${ROOT_CONFIG_EXECUTABLE} --incdir
    OUTPUT_VARIABLE ROOT_INCLUDE_DIR
    OUTPUT_STRIP_TRAILING_WHITESPACE)

  execute_process(
    COMMAND ${ROOT_CONFIG_EXECUTABLE} --libdir
    OUTPUT_VARIABLE ROOT_LIBRARY_DIR
    OUTPUT_STRIP_TRAILING_WHITESPACE)

  execute_process(
    COMMAND ${ROOT_CONFIG_EXECUTABLE} --libs
    OUTPUT_VARIABLE ROOT_LIBRARIES
    OUTPUT_STRIP_TRAILING_WHITESPACE)
    set (ROOT_LIBRARIES ${ROOT_LIBRARIES} -lGeom)

  execute_process(
    COMMAND ${ROOT_CONFIG_EXECUTABLE} --arch
    OUTPUT_VARIABLE ROOT_ARCH
    OUTPUT_STRIP_TRAILING_WHITESPACE)
endif()

# If search for root-config failed try to use directly user paths if set
#
if (NOT ROOT_FOUND)
  find_path(ROOT_INCLUDE_DIR NAMES TObject.h PATHS
    ${ROOT_INC_DIR}
    ${ROOT_DIR}/include
  )
  find_path(ROOT_LIBRARY_DIR NAMES libCore.so libCore.dylib PATHS
    ${ROOT_LIB_DIR}
    ${ROOT_LIB}/include
  )
  if (ROOT_INCLUDE_DIR AND ROOT_LIBRARY_DIR)
    set (ROOT_FOUND TRUE)
    set (ROOT_LIBRARIES -L${ROOT_LIBRARY_DIR} -lCore -lCint -lRIO -lNet -lHist -lGraf -lGraf3d -lGpad -lTree -lRint -lPostscript -lMatrix -lPhysics -lMathCore -lThread -pthread -lm -ldl -rdynamic -lGeom)
  endif()  
endif()    

if(ROOT_FOUND)
  set(LD_LIBRARY_PATH ${LD_LIBRARY_PATH} ${ROOT_LIBRARY_DIR})
  if(NOT ROOT_FIND_QUIETLY)
    message(STATUS "Found ROOT ${ROOT_VERSION} in ${ROOT_PREFIX}")
  endif()  
else()
  if (ROOT_FIND_REQUIRED)
    message(FATAL_ERROR "ROOT required, but not found")
  endif (ROOT_FIND_REQUIRED)   
endif()

# Make variables changeble to the advanced user
mark_as_advanced(ROOT_INCLUDE_DIR ROOT_LIBRARY_DIR ROOT_LIBRARIES )

#----------------------------------------------------------------------------
# Dictionary generation
include(CMakeMacroParseArguments)
find_program(ROOTCINT_EXECUTABLE rootcint PATHS
  $ROOT_DIR/bin
  $ENV{ROOTSYS}/bin
)

# According to root/cmake/modules/RootNewMacros.cmake 
# from Root v5.34.14
#---------------------------------------------------------------------------------------------------
#---ROOT_GENERATE_DICTIONARY( dictionary headerfiles LINKDEF linkdef OPTIONS opt1 opt2 ...)
#---------------------------------------------------------------------------------------------------
function(ROOT_GENERATE_DICTIONARY dictionary)
  PARSE_ARGUMENTS(ARG "LINKDEF;OPTIONS" "" ${ARGN})
  #---Get the list of header files-------------------------
  set(headerfiles)
  foreach(fp ${ARG_DEFAULT_ARGS})
    file(GLOB files inc/${fp})
    if(files)
      foreach(f ${files})
        if(NOT f MATCHES LinkDef)
          set(headerfiles ${headerfiles} ${f})
        endif()
      endforeach()
    else()
      set(headerfiles ${headerfiles} ${fp})
    endif()
  endforeach()
  string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}/include/" ""  rheaderfiles "${headerfiles}")
  #---Get the list of include directories------------------
  get_directory_property(incdirs INCLUDE_DIRECTORIES)
  if(CMAKE_PROJECT_NAME STREQUAL ROOT)
    set(includedirs -I${CMAKE_CURRENT_SOURCE_DIR}/include 
                    -I${CMAKE_BINARY_DIR}/include
                    -I${CMAKE_SOURCE_DIR}/cint/cint/include 
                    -I${CMAKE_SOURCE_DIR}/cint/cint/stl 
                    -I${CMAKE_SOURCE_DIR}/cint/cint/lib)
  else()
    set(includedirs -I${CMAKE_CURRENT_SOURCE_DIR}/include) 
  endif() 
  foreach( d ${incdirs})    
   set(includedirs ${includedirs} -I${d})
  endforeach()
  # filter-out -I/usr/include
  string(REPLACE "-I/usr/include" ""  includedirs "${includedirs}")
  #---Get the list of definitions---------------------------
  get_directory_property(defs COMPILE_DEFINITIONS)
  foreach( d ${defs})
   if(NOT d MATCHES "=")   
     set(definitions ${definitions} -D${d})
   endif()
  endforeach()
  #---Get LinkDef.h file------------------------------------
  foreach( f ${ARG_LINKDEF})
    if( IS_ABSOLUTE ${f})
      set(_linkdef ${_linkdef} ${f})
    else() 
      if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/include/${f})
        set(_linkdef ${_linkdef} ${CMAKE_CURRENT_SOURCE_DIR}/include/${f})
      else()
        set(_linkdef ${_linkdef} ${CMAKE_CURRENT_SOURCE_DIR}/${f})
      endif()
    endif()
  endforeach()
  #---call rootcint------------------------------------------
  #add_custom_command(OUTPUT ${dictionary}.cxx ${dictionary}.h
  #                   COMMAND ${ROOTCINT_EXECUTABLE} -cint -f  ${dictionary}.cxx 
  #                      -c ${ARG_OPTIONS} ${definitions} ${includedirs} ${rheaderfiles} ${_linkdef} 
  #                   DEPENDS ${headerfiles} ${_linkdef} ${ROOTCINTDEP})
  add_custom_command(OUTPUT ${dictionary}.cxx ${dictionary}.h
                     COMMAND ${ROOTCINT_EXECUTABLE} -cint -f  ${dictionary}.cxx 
                         -c ${ARG_OPTIONS} ${definitions} ${includedirs} ${rheaderfiles} ${_linkdef} 
                     DEPENDS ${headerfiles} ${_linkdef} ${ROOTCINTDEP})
endfunction()


