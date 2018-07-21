# A custom command and target to turn the OpenCL kernel into a byte array header
# The normal build depends on it properly and if the kernel file is changed, then
# a rebuild of libethash-cl should be triggered

#TODO: clean up the copy&pasting here
add_custom_command(
	OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/CLMiner_kernel_stable.h
	COMMAND ${CMAKE_COMMAND} ARGS
	-DBIN2H_SOURCE_FILE="${CMAKE_CURRENT_SOURCE_DIR}/CLMiner_kernel_stable.cl"
	-DBIN2H_VARIABLE_NAME=CLMiner_kernel_stable
	-DBIN2H_HEADER_FILE="${CMAKE_CURRENT_BINARY_DIR}/CLMiner_kernel_stable.h"
	-P "${CMAKE_CURRENT_SOURCE_DIR}/bin2h.cmake"
	COMMENT "Generating OpenCL Kernel Byte Array"
	DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/CLMiner_kernel_stable.cl
)
add_custom_target(clbin2h_stable DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/CLMiner_kernel_stable.h ${CMAKE_CURRENT_SOURCE_DIR}/CLMiner_kernel_stable.cl)

add_custom_command(
	OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/CLMiner_kernel_experimental.h
	COMMAND ${CMAKE_COMMAND} ARGS
	-DBIN2H_SOURCE_FILE="${CMAKE_CURRENT_SOURCE_DIR}/CLMiner_kernel_experimental.cl"
	-DBIN2H_VARIABLE_NAME=CLMiner_kernel_experimental
	-DBIN2H_HEADER_FILE="${CMAKE_CURRENT_BINARY_DIR}/CLMiner_kernel_experimental.h"
	-P "${CMAKE_CURRENT_SOURCE_DIR}/bin2h.cmake"
	COMMENT "Generating OpenCL Kernel Byte Array"
	DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/CLMiner_kernel_experimental.cl
)
add_custom_target(clbin2h_experimental DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/CLMiner_kernel_experimental.h ${CMAKE_CURRENT_SOURCE_DIR}/CLMiner_kernel_experimental.cl)

add_custom_command(
	OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/CLMiner_kernel_ethash_new.h
	COMMAND ${CMAKE_COMMAND} ARGS
	-DBIN2H_SOURCE_FILE="${CMAKE_CURRENT_SOURCE_DIR}/ethash-new.cl"
	-DBIN2H_VARIABLE_NAME=CLMiner_kernel_ethash_new
	-DBIN2H_HEADER_FILE="${CMAKE_CURRENT_BINARY_DIR}/CLMiner_kernel_ethash_new.h"
	-P "${CMAKE_CURRENT_SOURCE_DIR}/bin2h.cmake"
	COMMENT "Generating OpenCL Kernel Byte Array"
	DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/ethash-new.cl
)
add_custom_target(clbin2h_ethash_new DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/CLMiner_kernel_ethash_new.h ${CMAKE_CURRENT_SOURCE_DIR}/ethash-new.cl)

add_custom_command(
	OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/CLMiner_kernel_ethash_genoil.h
	COMMAND ${CMAKE_COMMAND} ARGS
	-DBIN2H_SOURCE_FILE="${CMAKE_CURRENT_SOURCE_DIR}/ethash-genoil.cl"
	-DBIN2H_VARIABLE_NAME=CLMiner_kernel_ethash_genoil
	-DBIN2H_HEADER_FILE="${CMAKE_CURRENT_BINARY_DIR}/CLMiner_kernel_ethash_genoil.h"
	-P "${CMAKE_CURRENT_SOURCE_DIR}/bin2h.cmake"
	COMMENT "Generating OpenCL Kernel Byte Array"
	DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/ethash-genoil.cl
)
add_custom_target(clbin2h_ethash_genoil DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/CLMiner_kernel_ethash_genoil.h ${CMAKE_CURRENT_SOURCE_DIR}/ethash-genoil.cl)

add_custom_command(
	OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/CLMiner_kernel_ethash_old.h
	COMMAND ${CMAKE_COMMAND} ARGS
	-DBIN2H_SOURCE_FILE="${CMAKE_CURRENT_SOURCE_DIR}/ethash.cl"
	-DBIN2H_VARIABLE_NAME=CLMiner_kernel_ethash_old
	-DBIN2H_HEADER_FILE="${CMAKE_CURRENT_BINARY_DIR}/CLMiner_kernel_ethash_old.h"
	-P "${CMAKE_CURRENT_SOURCE_DIR}/bin2h.cmake"
	COMMENT "Generating OpenCL Kernel Byte Array"
	DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/ethash.cl
)
add_custom_target(clbin2h_ethash_old DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/CLMiner_kernel_ethash_old.h ${CMAKE_CURRENT_SOURCE_DIR}/ethash-old.cl)

add_custom_command(
	OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/CLMiner_kernel_basic.h
	COMMAND ${CMAKE_COMMAND} ARGS
	-DBIN2H_SOURCE_FILE="${CMAKE_CURRENT_SOURCE_DIR}/CLMiner_kernel_basic.cl"
	-DBIN2H_VARIABLE_NAME=CLMiner_kernel_basic
	-DBIN2H_HEADER_FILE="${CMAKE_CURRENT_BINARY_DIR}/CLMiner_kernel_basic.h"
	-P "${CMAKE_CURRENT_SOURCE_DIR}/bin2h.cmake"
	COMMENT "Generating OpenCL Kernel Byte Array"
	DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/CLMiner_kernel_basic.cl
)
add_custom_target(clbin2h_basic DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/CLMiner_kernel_basic.h ${CMAKE_CURRENT_SOURCE_DIR}/CLMiner_kernel_basic.cl)

set(SOURCES
	CLMiner.h CLMiner.cpp
	${CMAKE_CURRENT_BINARY_DIR}/CLMiner_kernel_stable.h
	${CMAKE_CURRENT_BINARY_DIR}/CLMiner_kernel_experimental.h
	${CMAKE_CURRENT_BINARY_DIR}/CLMiner_kernel_ethash_new.h
	${CMAKE_CURRENT_BINARY_DIR}/CLMiner_kernel_ethash_genoil.h
	${CMAKE_CURRENT_BINARY_DIR}/CLMiner_kernel_ethash_old.h
	${CMAKE_CURRENT_BINARY_DIR}/CLMiner_kernel_basic.h
)

if(APPLE)
	# On macOS use system OpenCL library.
	find_package(OpenCL REQUIRED)
else()
	hunter_add_package(OpenCL)
	find_package(OpenCL CONFIG REQUIRED)
endif()

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(..)

add_library(ethash-cl ${SOURCES})
target_link_libraries(ethash-cl PUBLIC ethcore ethash::ethash)
target_link_libraries(ethash-cl PRIVATE OpenCL::OpenCL)
target_link_libraries(ethash-cl PRIVATE Boost::filesystem)
