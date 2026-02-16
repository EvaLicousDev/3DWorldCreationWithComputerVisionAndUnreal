add_subdirectory(TemplateCode)

add_library(
	ProcessingLib
	ColourSpaceVisualiser.h
	ColourSpaceVisualiser.cpp
	ImageProcessor.h 
    ImageProcessor.cpp
	"TemplateCode/CommonOperations.h"
    "SingleChannelHistogram.h"
 "HistorgramGraphCreator.h")

include_directories(${OpenCV_DIR} TemplateCode)
target_link_libraries(ProcessingLib ${OpenCV_LIBS})
target_include_directories(ProcessingLib PUBLIC)