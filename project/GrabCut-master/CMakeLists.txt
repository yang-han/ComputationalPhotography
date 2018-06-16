cmake_minimum_required(VERSION 3.5)
project(GrabCut)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES
        GrabCut.cpp GrabCut.h GMM.cpp GMM.h graph.h graph.cpp instances.inc block.h maxflow.cpp
        GCApplication.cpp GCApplication.h TrimapGenerator.cpp TrimapGenerator.h
        GlobalMatting.cpp GlobalMatting.h main.cpp )

find_package(OpenCV REQUIRED)

add_executable(GrabCut ${SOURCE_FILES})
target_link_libraries(GrabCut ${OpenCV_LIBS})