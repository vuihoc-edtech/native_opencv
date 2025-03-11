cmake_minimum_required(VERSION 3.4.1)

include_directories(../include)
add_library(lib_opencv SHARED IMPORTED)
set_target_properties(lib_opencv PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libopencv_java4.so)

# Thêm đường dẫn đến thư mục cJSON
include_directories(../ios/Classes/cjson)
add_library(cjson STATIC ../ios/Classes/cjson/cJSON.c)

find_library(log-lib log)
add_library(native_opencv SHARED ../ios/Classes/native_opencv.cpp)
target_link_libraries(native_opencv lib_opencv ${log-lib} cjson)

