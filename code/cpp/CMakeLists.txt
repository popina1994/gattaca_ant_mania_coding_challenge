cmake_minimum_required(VERSION 3.22)
project(GATTACA LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 23)

# add_compile_options(-Wall -Wextra -Wshadow -Wconversion -Wuninitialized -Wmissing-declarations -Wsign-compare)



option(UNIT_TEST "Build Unit tests" ON)

file(GLOB_RECURSE CPP_SOURCES
${PROJECT_SOURCE_DIR}/src/*.cpp
${PROJECT_SOURCE_DIR}/src/*.cxx
)

add_executable(${PROJECT_NAME} ${PROJECT_SOURCE_DIR}/src/main.cpp ${CPP_SOURCES})
target_compile_definitions(${PROJECT_NAME} PRIVATE LOG_ON=true)

target_compile_options(${PROJECT_NAME} PRIVATE -g -O2 -fno-omit-frame-pointer )

target_include_directories(${PROJECT_NAME} PRIVATE include)


