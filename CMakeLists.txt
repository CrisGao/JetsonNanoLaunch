cmake_minimum_required(VERSION 2.8)

project(nanoSerial)

find_package(Threads)
find_package(OpenCV REQUIRED)

#如下为新加的
include(CheckCXXCompilerFlag) 
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11) 
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X) 

if(COMPILER_SUPPORTS_CXX11) 
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11") 
elseif(COMPILER_SUPPORTS_CXX0X) 
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x") 
else() 
	message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.") 
endif() 
#如上为新加的
set(CMAKE_CXX_STANDARD 11)

set(CMAKE_CXX_STANDARD_REQUIRED true)
set(OpenCV_DIR /usr/local/share/OpenCV)

include_directories(./include 
		     /home/george/caffe/include #caffe路径 
		     /usr/local/cuda/include
		     /home/george/caffe/build/src
		     ${OpenCV_INCLUDE_DIRS})

aux_source_directory(./sources DIR_SRCS)

#设置debug模式,当需要用vscode的时候开启
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

add_executable(nanoSerial ${DIR_SRCS})

target_link_libraries(nanoSerial 
			/home/george/caffe/build/lib/libcaffe.so
			/usr/lib/aarch64-linux-gnu/libglog.so
			/usr/lib/aarch64-linux-gnu/libboost_system.so
		        ${CMAKE_THREAD_LIBS_INIT}
			${OpenCV_LIBS})

