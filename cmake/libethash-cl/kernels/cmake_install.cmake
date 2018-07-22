# Install script for directory: /home/xlwolf/ethminer/modified/220718/ethminer/libethash-cl/kernels

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
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
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

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin/kernels" TYPE FILE FILES
    "/home/xlwolf/ethminer/modified/220718/ethminer/libethash-cl/kernels/bin/ethash_baffin_lws64.bin"
    "/home/xlwolf/ethminer/modified/220718/ethminer/libethash-cl/kernels/bin/ethash_tonga_lws64.bin"
    "/home/xlwolf/ethminer/modified/220718/ethminer/libethash-cl/kernels/bin/ethash_gfx901_lws64.bin"
    "/home/xlwolf/ethminer/modified/220718/ethminer/libethash-cl/kernels/bin/ethash_tonga_lws256.bin"
    "/home/xlwolf/ethminer/modified/220718/ethminer/libethash-cl/kernels/bin/ethash_tonga_lws192.bin"
    "/home/xlwolf/ethminer/modified/220718/ethminer/libethash-cl/kernels/bin/ethash_gfx901_lws128.bin"
    "/home/xlwolf/ethminer/modified/220718/ethminer/libethash-cl/kernels/bin/ethash_gfx901_lws256.bin"
    "/home/xlwolf/ethminer/modified/220718/ethminer/libethash-cl/kernels/bin/ethash_baffin_lws256.bin"
    "/home/xlwolf/ethminer/modified/220718/ethminer/libethash-cl/kernels/bin/ethash_baffin_lws192.bin"
    "/home/xlwolf/ethminer/modified/220718/ethminer/libethash-cl/kernels/bin/ethash_ellesmere_lws256.bin"
    "/home/xlwolf/ethminer/modified/220718/ethminer/libethash-cl/kernels/bin/ethash_ellesmere_lws192.bin"
    "/home/xlwolf/ethminer/modified/220718/ethminer/libethash-cl/kernels/bin/ethash_tonga_lws128.bin"
    "/home/xlwolf/ethminer/modified/220718/ethminer/libethash-cl/kernels/bin/ethash_ellesmere_lws128.bin"
    "/home/xlwolf/ethminer/modified/220718/ethminer/libethash-cl/kernels/bin/ethash_gfx901_lws192.bin"
    "/home/xlwolf/ethminer/modified/220718/ethminer/libethash-cl/kernels/bin/ethash_baffin_lws128.bin"
    "/home/xlwolf/ethminer/modified/220718/ethminer/libethash-cl/kernels/bin/ethash_ellesmere_lws64.bin"
    )
endif()

