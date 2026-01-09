cmake_minimum_required(VERSION 3.20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")

project(vanguard)
enable_language(CXX)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(
	/W4
	/permissive-
	/EHsc
)

add_compile_definitions(
	NOMINMAX
	WIN32_LEAN_AND_MEAN
	_CRT_SECURE_NO_WARNINGS
	_CRT_NONSTDC_NO_WARNINGS
)

add_executable(
	${PROJECT_NAME}
	src/backends/dx12/device.cpp
	src/gfx/device.cpp
	src/gfx/render_pass.cpp
	src/gfx/renderer.cpp
	src/gfx/shader.cpp
	src/app.cpp
	src/main.cpp
)

find_package(SDL3 REQUIRED)

add_subdirectory(extern/nvrhi)
add_subdirectory(extern/glm)

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/include
	${CMAKE_CURRENT_SOURCE_DIR}/src
	extern/glm
)

target_link_libraries(
	${PROJECT_NAME} PRIVATE
	SDL3::SDL3
	nvrhi_d3d12
	nvrhi
	dxgi
	d3d12
)

if (CMAKE_IMPORT_LIBRARY_SUFFIX)
	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:${PROJECT_NAME}> $<TARGET_FILE_DIR:${PROJECT_NAME}>
		COMMAND_EXPAND_LISTS
	)
endif ()

add_custom_target(run
	COMMAND ${PROJECT_NAME}
	DEPENDS ${PROJECT_NAME}
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

find_program(DXC_EXECUTABLE dxc REQUIRED)

set(SHADER_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/shaders)
set(SHADER_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/shaders)

file(GLOB_RECURSE SHADER_FILES
	CONFIGURE_DEPENDS
	${SHADER_SOURCE_DIR}/*.hlsl
)

set(COMPILED_SHADERS)

foreach (SHADER ${SHADER_FILES})
	file(RELATIVE_PATH REL_PATH ${SHADER_SOURCE_DIR} ${SHADER})
	get_filename_component(SHADER_NAME ${REL_PATH} NAME_WE)
	get_filename_component(SHADER_DIR ${REL_PATH} DIRECTORY)

	set(OUT_DIR ${SHADER_OUTPUT_DIR}/${SHADER_DIR})
	file(MAKE_DIRECTORY ${OUT_DIR})

	string(TOLOWER ${SHADER} SHADER_LOWER)

	if (SHADER_LOWER MATCHES "\\.vs")
		set(OUT_FILE ${OUT_DIR}/${SHADER_NAME}.vs.dxil)

		add_custom_command(
			OUTPUT ${OUT_FILE}
			COMMAND ${DXC_EXECUTABLE}
			-T vs_6_6
			-E VSmain
			-Fo ${OUT_FILE}
			${SHADER}
			DEPENDS ${SHADER}
			COMMENT "Compiling VS ${REL_PATH}"
		)

		list(APPEND COMPILED_SHADERS ${OUT_FILE})
	elseif (SHADER_LOWER MATCHES "\\.ps")
		set(OUT_FILE ${OUT_DIR}/${SHADER_NAME}.ps.dxil)

		add_custom_command(
			OUTPUT ${OUT_FILE}
			COMMAND ${DXC_EXECUTABLE}
			-T ps_6_6
			-E PSmain
			-Fo ${OUT_FILE}
			${SHADER}
			DEPENDS ${SHADER}
			COMMENT "Compiling PS ${REL_PATH}"
		)

		list(APPEND COMPILED_SHADERS ${OUT_FILE})
	else ()
		set(OUT_FILE_VS ${OUT_DIR}/${SHADER_NAME}.vs.dxil)
		set(OUT_FILE_PS ${OUT_DIR}/${SHADER_NAME}.ps.dxil)

		add_custom_command(
			OUTPUT ${OUT_FILE_VS}
			COMMAND ${DXC_EXECUTABLE}
			-T vs_6_6
			-E VSmain
			-Fo ${OUT_FILE_VS}
			${SHADER}
			DEPENDS ${SHADER}
			COMMENT "Compiling VS ${REL_PATH}"
		)

		add_custom_command(
			OUTPUT ${OUT_FILE_PS}
			COMMAND ${DXC_EXECUTABLE}
			-T ps_6_6
			-E PSmain
			-Fo ${OUT_FILE_PS}
			${SHADER}
			DEPENDS ${SHADER}
			COMMENT "Compiling PS ${REL_PATH}"
		)

		list(APPEND COMPILED_SHADERS ${OUT_FILE_VS} ${OUT_FILE_PS})
	endif ()
endforeach ()

add_custom_target(shaders ALL
	DEPENDS ${COMPILED_SHADERS}
)

add_dependencies(${PROJECT_NAME} shaders)
