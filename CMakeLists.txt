# CMakeList.txt : CMake project for OpenCVLegoMapScannerV1, include source and define
# project specific logic here.
cmake_minimum_required (VERSION 3.10)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project (OpenCVLegoMapScannerV1)
set( CMAKE_CXX_STANDARD 20 )
add_definitions(-DIMAGE_DIR="${CVDrive}") #this is where the app will take the images it's processing from 
include_directories(${CVDrive})

#computer vision library
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_DIR})

# Add source to this project's executable and additional files
# Include all 
add_subdirectory(src)
add_executable (OpenCVLegoMapScannerV1 "OpenCVLegoMapScannerV1.cpp")
target_link_libraries(OpenCVLegoMapScannerV1 PUBLIC ${OpenCV_LIBS} ProcessingLib)

#specify including subdirectories for parent projects
target_include_directories(OpenCVLegoMapScannerV1 PUBLIC)