cmake_minimum_required(VERSION 3.9)
project(grabcut)

set(CMAKE_CXX_STANDARD 11)


add_executable(grabcut
        main.cpp
        WindowFramework.cpp
        WindowFramework.h
        GaussMixtureModel.cpp
        GaussMixtureModel.h
        GrabCut.cpp
        GrabCut.h
        KMeansImpl.cpp
        KMeansImpl.h
        BorderMatting.cpp
        BorderMatting.h
        Dinic.cpp 
        Dinic.h)

find_package( OpenCV REQUIRED )
include_directories(${OpenCV_INCLUDE_DIRS})
target_link_libraries( grabcut ${OpenCV_LIBS} )