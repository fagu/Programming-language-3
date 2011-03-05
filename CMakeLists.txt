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
	opcodes.cpp
	opcodes_compiler.cpp
	flowgraph.cpp
	bison_parser.cpp
	flex_lexer.cpp
)

set(VM_CPPFILES
	virtualmachine.cpp
	garbagecollector.cpp
	opcodes.cpp
)

add_executable(compiler ${COMPILER_CPPFILES})
add_executable(vm ${VM_CPPFILES})

target_link_libraries(compiler fl)

ENABLE_TESTING()

ADD_TEST(performance ../testcase.sh performance)
ADD_TEST(array ../testcase.sh array)
ADD_TEST(functions ../testcase.sh functions)
ADD_TEST(linkedlist ../testcase.sh linkedlist)
ADD_TEST(string ../testcase.sh string)
ADD_TEST(vector ../testcase.sh vector)
