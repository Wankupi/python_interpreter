### YOU CAN'T MODIFY THE CODE BELOW
cmake_minimum_required(VERSION 3.15.2)
project(python_interpreter)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Ofast")

include_directories(/usr/include/antlr4-runtime/)
include_directories(
	${PROJECT_SOURCE_DIR}/generated/
	${PROJECT_SOURCE_DIR}/src/
)

add_subdirectory(${PROJECT_SOURCE_DIR}/generated)
### YOU CAN"T MODIFY THE CODE ABOVE

file(GLOB_RECURSE main_src src/*.cpp)

add_executable(code ${main_src}) # Add all *.cpp file after src/main.cpp, like src/Evalvisitor.cpp did

### YOU CAN'T MODIFY THE CODE BELOW
target_link_libraries(code PyAntlr)
target_link_libraries(code antlr4-runtime)
### YOU CAN"T MODIFY THE CODE ABOVE


enable_testing()
file(GLOB_RECURSE tests testcases/*.in)
foreach(test ${tests})
	get_filename_component(test_name ${test} NAME_WE)
	string(REPLACE ".in" ".out" answer "${test}")
	message(STATUS "Adding test ${test_name} ${answer}")
	add_test(
		NAME ${test_name}
		COMMAND bash -c "${PROJECT_SOURCE_DIR}/run.sh $<TARGET_FILE:code> ${test} ${answer} ${test_name}.out"
		WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/run/
	)
endforeach ()
