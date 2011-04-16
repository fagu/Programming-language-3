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
	flowgraph.cpp
	function.cpp
	environment.cpp
	runner.cpp
	garbagecollector.cpp
	bison_parser.cpp
	flex_lexer.cpp
)

set(VM_CPPFILES
	virtualmachine.cpp
	garbagecollector.cpp
	opcodes.cpp
	runner.cpp
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
ADD_TEST(globals ../testcase.sh globals)

find_package(Doxygen)
if(DOXYGEN_FOUND)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
add_custom_target(doc
${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
COMMENT "Generating API documentation with Doxygen" VERBATIM
)
endif(DOXYGEN_FOUND)
