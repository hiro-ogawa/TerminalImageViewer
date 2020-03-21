cmake_minimum_required(VERSION 3.8)
project(termcam)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
find_package(OpenCV REQUIRED)  

include_directories( ${OpenCV_INCLUDE_DIRS} )

add_executable(tiv_cv tiv_cv.cpp libtiv.cpp)
target_link_libraries(tiv_cv ${OpenCV_LIBS})
