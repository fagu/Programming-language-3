cmake_minimum_required (VERSION 2.6)
project(programmiersprache3)

include (bison.cmake)

add_flex_files(lexer.flex)
add_bison_files(parser.y)

set(CPPFILES
	main.cpp
	type.cpp
	parseresult.cpp
	instruction.cpp
	bison_parser.cpp
	flex_lexer.cpp
)

add_executable(programmiersprache3 ${CPPFILES})

target_link_libraries(programmiersprache3 fl)
