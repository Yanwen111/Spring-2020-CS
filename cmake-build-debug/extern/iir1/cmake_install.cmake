# Install script for directory: /home/yanwen/CML_CS/Summer 2020/Spring-2020-CS/extern/iir1

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Debug")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  foreach(file
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libiir.so.1.7.7"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libiir.so.1"
      )
    if(EXISTS "${file}" AND
       NOT IS_SYMLINK "${file}")
      file(RPATH_CHECK
           FILE "${file}"
           RPATH "")
    endif()
  endforeach()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES
    "/home/yanwen/CML_CS/Summer 2020/Spring-2020-CS/cmake-build-debug/extern/iir1/libiir.so.1.7.7"
    "/home/yanwen/CML_CS/Summer 2020/Spring-2020-CS/cmake-build-debug/extern/iir1/libiir.so.1"
    )
  foreach(file
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libiir.so.1.7.7"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libiir.so.1"
      )
    if(EXISTS "${file}" AND
       NOT IS_SYMLINK "${file}")
      if(CMAKE_INSTALL_DO_STRIP)
        execute_process(COMMAND "/usr/bin/strip" "${file}")
      endif()
    endif()
  endforeach()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libiir.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libiir.so")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libiir.so"
         RPATH "")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES "/home/yanwen/CML_CS/Summer 2020/Spring-2020-CS/cmake-build-debug/extern/iir1/libiir.so")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libiir.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libiir.so")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libiir.so")
    endif()
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/iir" TYPE FILE FILES
    "/home/yanwen/CML_CS/Summer 2020/Spring-2020-CS/extern/iir1/iir/Biquad.h"
    "/home/yanwen/CML_CS/Summer 2020/Spring-2020-CS/extern/iir1/iir/Butterworth.h"
    "/home/yanwen/CML_CS/Summer 2020/Spring-2020-CS/extern/iir1/iir/Cascade.h"
    "/home/yanwen/CML_CS/Summer 2020/Spring-2020-CS/extern/iir1/iir/ChebyshevI.h"
    "/home/yanwen/CML_CS/Summer 2020/Spring-2020-CS/extern/iir1/iir/ChebyshevII.h"
    "/home/yanwen/CML_CS/Summer 2020/Spring-2020-CS/extern/iir1/iir/Common.h"
    "/home/yanwen/CML_CS/Summer 2020/Spring-2020-CS/extern/iir1/iir/Custom.h"
    "/home/yanwen/CML_CS/Summer 2020/Spring-2020-CS/extern/iir1/iir/Layout.h"
    "/home/yanwen/CML_CS/Summer 2020/Spring-2020-CS/extern/iir1/iir/MathSupplement.h"
    "/home/yanwen/CML_CS/Summer 2020/Spring-2020-CS/extern/iir1/iir/PoleFilter.h"
    "/home/yanwen/CML_CS/Summer 2020/Spring-2020-CS/extern/iir1/iir/RBJ.h"
    "/home/yanwen/CML_CS/Summer 2020/Spring-2020-CS/extern/iir1/iir/State.h"
    "/home/yanwen/CML_CS/Summer 2020/Spring-2020-CS/extern/iir1/iir/Types.h"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include" TYPE FILE FILES "/home/yanwen/CML_CS/Summer 2020/Spring-2020-CS/extern/iir1/Iir.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "/home/yanwen/CML_CS/Summer 2020/Spring-2020-CS/cmake-build-debug/extern/iir1/libiir_static.a")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/iir" TYPE FILE FILES
    "/home/yanwen/CML_CS/Summer 2020/Spring-2020-CS/extern/iir1/iir/Biquad.h"
    "/home/yanwen/CML_CS/Summer 2020/Spring-2020-CS/extern/iir1/iir/Butterworth.h"
    "/home/yanwen/CML_CS/Summer 2020/Spring-2020-CS/extern/iir1/iir/Cascade.h"
    "/home/yanwen/CML_CS/Summer 2020/Spring-2020-CS/extern/iir1/iir/ChebyshevI.h"
    "/home/yanwen/CML_CS/Summer 2020/Spring-2020-CS/extern/iir1/iir/ChebyshevII.h"
    "/home/yanwen/CML_CS/Summer 2020/Spring-2020-CS/extern/iir1/iir/Common.h"
    "/home/yanwen/CML_CS/Summer 2020/Spring-2020-CS/extern/iir1/iir/Custom.h"
    "/home/yanwen/CML_CS/Summer 2020/Spring-2020-CS/extern/iir1/iir/Layout.h"
    "/home/yanwen/CML_CS/Summer 2020/Spring-2020-CS/extern/iir1/iir/MathSupplement.h"
    "/home/yanwen/CML_CS/Summer 2020/Spring-2020-CS/extern/iir1/iir/PoleFilter.h"
    "/home/yanwen/CML_CS/Summer 2020/Spring-2020-CS/extern/iir1/iir/RBJ.h"
    "/home/yanwen/CML_CS/Summer 2020/Spring-2020-CS/extern/iir1/iir/State.h"
    "/home/yanwen/CML_CS/Summer 2020/Spring-2020-CS/extern/iir1/iir/Types.h"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include" TYPE FILE FILES "/home/yanwen/CML_CS/Summer 2020/Spring-2020-CS/extern/iir1/Iir.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/iir/iir-config.cmake")
    file(DIFFERENT EXPORT_FILE_CHANGED FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/iir/iir-config.cmake"
         "/home/yanwen/CML_CS/Summer 2020/Spring-2020-CS/cmake-build-debug/extern/iir1/CMakeFiles/Export/lib/cmake/iir/iir-config.cmake")
    if(EXPORT_FILE_CHANGED)
      file(GLOB OLD_CONFIG_FILES "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/iir/iir-config-*.cmake")
      if(OLD_CONFIG_FILES)
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/iir/iir-config.cmake\" will be replaced.  Removing files [${OLD_CONFIG_FILES}].")
        file(REMOVE ${OLD_CONFIG_FILES})
      endif()
    endif()
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/iir" TYPE FILE FILES "/home/yanwen/CML_CS/Summer 2020/Spring-2020-CS/cmake-build-debug/extern/iir1/CMakeFiles/Export/lib/cmake/iir/iir-config.cmake")
  if("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/iir" TYPE FILE FILES "/home/yanwen/CML_CS/Summer 2020/Spring-2020-CS/cmake-build-debug/extern/iir1/CMakeFiles/Export/lib/cmake/iir/iir-config-debug.cmake")
  endif()
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("/home/yanwen/CML_CS/Summer 2020/Spring-2020-CS/cmake-build-debug/extern/iir1/test/cmake_install.cmake")
  include("/home/yanwen/CML_CS/Summer 2020/Spring-2020-CS/cmake-build-debug/extern/iir1/demo/cmake_install.cmake")

endif()

