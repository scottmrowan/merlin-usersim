macro (merlin_test dir exe_name)
	set(source_files "")
	foreach(arg IN ITEMS ${ARGN})
		list(APPEND source_files ${dir}/${arg})
	endforeach()
	add_executable(${exe_name} ${source_files})

	# To keep the directory structure, we must set the output_name, add a rule to make the directory
	set_target_properties( ${exe_name} PROPERTIES OUTPUT_NAME ${dir}/${exe_name} )
	set_target_properties( ${exe_name} PROPERTIES COMPILE_FLAGS -g )
	add_custom_command(TARGET ${exe_name} PRE_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory ${dir})
	target_link_libraries(${exe_name} merlin)

endmacro (merlin_test)


add_custom_target(TestDataFiles ALL)
add_custom_command(TARGET TestDataFiles
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                   ${CMAKE_CURRENT_SOURCE_DIR}/data ${CMAKE_CURRENT_BINARY_DIR}/data)


merlin_test(BasicTests bunch_test bunch_test.cpp)
add_test(bunch_test BasicTests/bunch_test)

merlin_test(BasicTests landau_test landau_test.cpp)
add_test(landau_test BasicTests/landau_test WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/)

merlin_test(OpticsTests lhc_optics_test lhc_optics_test.cpp)
add_test(lhc_optics_test OpticsTests/lhc_optics_test WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/)

find_program(MEMORYCHECK_COMMAND NAMES valgrind)
INCLUDE(Dart)

