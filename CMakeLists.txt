cmake_minimum_required(VERSION 3.17)
project(UnivProject)

set(CMAKE_CXX_STANDARD 17)

add_executable(UnivProject main.cpp MyDataBase/TextDataBase.cpp MyDataBase/TextDataBase.h)