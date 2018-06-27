cmake_minimum_required(VERSION 2.4)
add_library (GstPG opencv_gstreamer.cpp)
find_package(OpenCV REQUIRED)
target_link_libraries(GstPG ${OpenCV_LIBS})
target_include_directories (flycapture PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/flycapture)
add_library (flycapture Flycapture2.h)
target_include_directories (GstPG PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
add_executable (GstPg opencv_gstreamer.cpp)
target_link_libraries (GstPg LINK_PUBLIC GstPG)


