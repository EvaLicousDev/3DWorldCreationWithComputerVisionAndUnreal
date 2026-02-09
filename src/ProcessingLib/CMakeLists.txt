add_library(
	ProcessingLib
	ImageProcessor.h 
    ImageProcessor.cpp
)

include_directories(${OpenCV_DIR})
target_link_libraries(ProcessingLib ${OpenCV_LIBS})
target_include_directories(ProcessingLib PUBLIC)