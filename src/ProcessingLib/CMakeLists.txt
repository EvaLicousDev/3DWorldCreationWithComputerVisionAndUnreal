add_subdirectory(BrickCVEnums)
add_subdirectory(ERRORs)
add_subdirectory(Histogram)
add_subdirectory(TemplateCode)

add_library(
	ProcessingLib
	ImageProcessor.h 
    ImageProcessor.cpp
	TemplateCode/CommonOperations.h
	ERRORs/ErrorTypes.h
	BrickCVEnums/BrickColourEnum.h
	BrickCVEnums/ChannelType.h
	Histogram/SingleChannelHistogram.h
	Histogram/ColourHistogram.h
	Histogram/ColourHistogram.cpp
	Histogram/HistorgramGraphCreator.h
  "ERRORs/ErrorOutput.h" "BrickColourClassifier.h" "CustomHDRProcessing/HDRImageArrayMerger.h" "CustomHDRProcessing/HDRImageArrayMerger.cpp")

include_directories(${OpenCV_DIR})
target_link_libraries(ProcessingLib ${OpenCV_LIBS})
target_include_directories(ProcessingLib PUBLIC)