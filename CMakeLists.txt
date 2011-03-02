cmake_minimum_required (VERSION 2.6)
project(programmiersprache3)

include (bison.cmake)

add_flex_files(lexer.flex)
add_bison_files(parser.y)

set(COMPILER_CPPFILES
	main.cpp
	type.cpp
	parseresult.cpp
	instruction.cpp
	location.cpp
	bison_parser.cpp
	flex_lexer.cpp
)

set(VM_CPPFILES
	virtualmachine.cpp
	garbagecollector.cpp
)

add_executable(compiler ${COMPILER_CPPFILES})
add_executable(vm ${VM_CPPFILES})

target_link_libraries(compiler fl)
