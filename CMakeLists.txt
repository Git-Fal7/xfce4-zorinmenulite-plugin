cmake_minimum_required(VERSION 2.8)

project(zorinmenulite)

# version number
set(zorinmenulite_version_major "1")
set(zorinmenulite_version_minor "1")
set(zorinmenulite_version_micro "3")
set(zorinmenulite_version_tag "")
set(zorinmenulite_version "${zorinmenulite_version_major}.${zorinmenulite_version_minor}.${zorinmenulite_version_micro}")
if(${zorinmenulite_version_tag} MATCHES "git")
	if(NOT DEFINED zorinmenulite_version_build)
		execute_process(COMMAND git describe
			WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
			OUTPUT_VARIABLE zorinmenulite_version_build)
		string(REGEX REPLACE "^.*-g" "" zorinmenulite_version_build "${zorinmenulite_version_build}")
		string(REGEX REPLACE "\n" "" zorinmenulite_version_build "${zorinmenulite_version_build}")
	endif()
	set(zorinmenulite_version "${zorinmenulite_version}.${zorinmenulite_version_tag}-${zorinmenulite_version_build}")
endif()

# options
option(ENABLE_VISIBILITY "Enable ELF visibility attributes" ON)
option(ENABLE_AS_NEEDED "Enable -Wl,--as-needed for the linker" ON)
option(ENABLE_LINKER_OPTIMIZED_HASH_TABLES "Enable -Wl,-O1 for the linker" ON)
option(ENABLE_DEVELOPER_MODE "Enable strict checks to help with development" OFF)

include(GNUInstallDirs)

add_subdirectory(icons)
add_subdirectory(panel-plugin)
add_subdirectory(po)

# uninstall target
configure_file(
	${CMAKE_CURRENT_SOURCE_DIR}/cmake/uninstall.cmake.in
	${CMAKE_CURRENT_BINARY_DIR}/cmake/uninstall.cmake
	IMMEDIATE @ONLY)
add_custom_target(uninstall
	${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake/uninstall.cmake)
