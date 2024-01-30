cmake_minimum_required(VERSION 3.5)

set(TARGET_NAME truthsystem)
project(${TARGET_NAME} LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置moc rcc uic
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

include_directories(ui)
include_directories(if)
include_directories(ars_40x)
include_directories(sensor_fusion)
include_directories(./)

# 添加qt5子模块
set(CMAKE_PREFIX_PATH "/opt/Qt5.12.8/5.12.8/gcc_64")
find_package(Qt5 5.12.8 EXACT COMPONENTS Core PrintSupport Gui Widgets OpenGL Xml WebSockets Concurrent REQUIRED Network)

FILE(GLOB ${TARGET_NAME}_SOURCES  "./if/*.cpp" "./if/*.c" "./if/*.cc"
                                  "./*.cpp" "./*.c" "./*.cc"
                                  "./ui/*.cpp" "./ui/*.c" "./ui/*.cc"
                                  "./ars_40x/*.cpp" "./ars_40x/*.c" "./ars_40x/*.cc"
                                  "./utils/*.cpp" "./utils/*.c" "./utils/*.cc"
                                  "./sensor_fusion/*.cpp" "./sensor_fusion/*.c" "./sensor_fusion/*.cc")

FILE(GLOB ${TARGET_NAME}_HEADER "./if/*.h" "./if/*.hpp"
                                "./*.h" "./*.hpp"  
                                "./ui/*.h" "./ui/*.hpp"
                                "./ars_40x/*.h" "./ars_40x/*.hpp"
                                "./utils/*.h" "./utils/*.hpp"
                                "./sensor_fusion/*.h" "./sensor_fusion/*.hpp")

FILE(GLOB ${TARGET_NAME}_UI "./ui/*.ui")       

qt5_add_resources(QRC_FILES source.qrc ./res/skin/dark/darkstyle.qrc ./res/skin/light/lightstyle.qrc)

set(SOURCE_FILES
  # main.cpp
  # common.h
  # ./if/ArtosynNet.h
  # ./if/ArtosynNet.cpp
  # ./if/Encode2H264.h
  # ./if/Encode2H264.cpp
  # ./if/h264decode.cpp
  # ./if/h264decode.h
  # ./ui/mainwindow.cpp
  # ./ui/mainwindow.h
  # ./ui/cameraparamwidget.cpp
  # ./ui/cameraparamwidget.h
  # ./ui/systemparamwidget.cpp
  # ./ui/systemparamwidget.h
  # ./ui/systemparamwidget.cpp
  # ./ui/systemparamwidget.h
  # ./ui/cameradiswidget.cpp
  # ./ui/cameradiswidget.h
  # ./ui/transconfig.cpp
  # ./ui/transconfig.h
  # ./ui/replaywidget.cpp
  # ./ui/replaywidget.h
  # ./ui/recordconfig.cpp
  # ./ui/recordconfig.h
  # ./ui/systemsetwidget.cpp
  # ./ui/systemsetwidget.h
  # ./ui/plotviewer.cpp
  # ./ui/plotviewer.h
  # ./ui/eventwidget.cpp
  # ./ui/eventwidget.h
  # ./ui/playbackui.cpp
  # ./ui/playbackui.h
  # ./ui/videowidget.cpp
  # ./ui/videowidget.h
  # ./ui/lidarviewerwidget.cpp
  # ./ui/lidarviewerwidget.h
  # ./ui/lidarconnectwidget.cpp
  # ./ui/lidarconnectwidget.h
  # ./ui/sensorfusion.cpp
  # ./ui/sensorfusion.h

  # memcache.cpp
  # memcache.h
  # GLYuvWidget.cpp
  # GLYuvWidget.h
  # alg_cvtColor.cpp
  # alg_cvtColor.h
  # sharedmembuffer.cpp
  # sharedmembuffer.h
  # MyQOpenglWidget.cpp
  # MyQOpenglWidget.h
  # minboundingbox.cpp
  # minboundingbox.h
  # recorder.cpp
  # recorder.h
  # pandersdk.cpp
  # pandersdk.h

  # lidar_AT128.cpp
  # lidar_AT128.h
  # inisettings.cpp
  # inisettings.h
  # configinfo.cpp
  # configinfo.h
  # initeractionbypcie.cpp
  # initeractionbypcie.h
  # radar_ars408.cpp
  # radar_ars408.h
  # qcustomplot.cpp
  # qcustomplot.h
  # usbcan_alyst-II.hpp
  # ConStatus.hpp
  # initeractionbyhttp.cpp
  # initeractionbyhttp.h
  # HttpClient.cpp
  # HttpClient.h
  # h264play.cpp
  # h264play.h
  # playfactory.cpp
  # playfactory.h
  # yuvplay.cpp
  # yuvplay.h
  # nvs_result.h
  # constdef.h
  # util.cpp
  # util.h
  # videoinfo.cpp
  # videoinfo.h
  # StaticSignals.h
  # nvshandler.cpp
  # nvshandler.h
  # miximage.cpp
  # miximage.h
  # blockingqueue.h
  # qttoopencv.cpp
  # qttoopencv.h
  # cremap.h
  # cremap.cpp
  # imuhandler.h
  # imuhandler.cpp
  # imu.cpp
  # imu.h
  # glmyyuvwidget.cpp
  # glmyyuvwidget.h
  # mapping.cpp
  # mapping.h
  # yuv2rgb.cpp
  # yuv2rgb.h
  # viewer.cpp
  # viewer.h

  # ./ars_40x/ars_40x_can.cc
  # ./ars_40x/ars_40x_can.h
  # ./ars_40x/cluster_list.hpp
  # ./ars_40x/object_list.hpp
  # ./ars_40x/radar_Cfg.hpp
  # ./ars_40x/radar_state.hpp



  # utils/GQueue.h
  # utils/cCSV.hpp
  # utils/global.hpp
  # object.h
  # array.h
  # log.cpp
  # log.h
  # vehviewer.cpp
  # vehviewer.h
  # mathtypes.cpp
  # mathtypes.h
  # define.h
  # singleton.h
  # MapNew.h
  # MapNew.cpp
  # ./ui/videopaintlabel.h
  # ./ui/videopaintlabel.cpp



  # ./ui/mainwindow.ui
  # ./ui/cameraparamwidget.ui
  # ./ui/systemparamwidget.ui
  # ./ui/cameradiswidget.ui
  # ./ui/transconfig.ui
  # ./ui/recordconfig.ui
  # ./ui/replaywidget.ui
  # ./ui/systemsetwidget.ui
  # ./ui/eventwidget.ui
  # ./ui/playbackui.ui
  # ./ui/videowidget.ui
  # ./ui/lidarviewerwidget.ui
  # ./ui/lidarconnectwidget.ui
)


add_executable(${TARGET_NAME}
  ${QRC_FILES}
  ${${TARGET_NAME}_SOURCES}
  ${${TARGET_NAME}_HEADER}
  ${${TARGET_NAME}_UI}
)

# 将Qt库链接到可执行文件
target_link_libraries(${TARGET_NAME} PRIVATE Qt5::Core Qt5::Gui Qt5::Widgets Qt5::Xml Qt5::OpenGL Qt5::Network Qt5::WebSockets Qt5::PrintSupport Qt5::Concurrent)

# opencv
if(WIN32)
#  set(OpenCV_DIR "D:/CompileData/CMake_Data/calibration/calibration/lib/opencv/win/opencv-3.4.1/lib")
#  find_package(OpenCV 3.4.1 REQUIRED )
  target_include_directories(${TARGET_NAME} PRIVATE "./tools/opencv/win/opencv-3.4.1/include")
  target_link_directories(${TARGET_NAME} PRIVATE "./tools/opencv/win/opencv-3.4.1/lib")
  string(TOLOWER ${CMAKE_BUILD_TYPE} BUILD_TYPE)
  if(BUILD_TYPE STREQUAL "debug")
	target_link_libraries(${TARGET_NAME} PRIVATE opencv_world341d)
  elseif(BUILD_TYPE STREQUAL "release")
	target_link_libraries(${TARGET_NAME} PRIVATE opencv_world341)
  endif()
  
#  message(STATUS "OpenCV_INCLUDE_DIRS=${OpenCV_INCLUDE_DIRS}")
elseif(UNIX)
  #set(OpenCV_DIR ./tools/opencv/linux/opencv-3.4.16/opencv/share/OpenCV/)
  set(OpenCV_DIR /usr/local/opencv3.4.10/share/OpenCV)
  find_package(OpenCV REQUIRED )
  target_include_directories(${TARGET_NAME} PRIVATE ${OpenCV_INCLUDE_DIRS})
  target_link_libraries(${TARGET_NAME} PRIVATE ${OpenCV_LIBS})
  message(STATUS "OpenCV_INCLUDE_DIRS=${OpenCV_LIBS}")

#  target_include_directories(${TARGET_NAME} PRIVATE "/usr/lib/include")
#  target_link_directories(${TARGET_NAME} PRIVATE "/usr/lib/lib")
endif()

# PCL库
find_package(PCL REQUIRED COMPONENTS 
  common features io kdtree ml octree search
)
target_include_directories(${TARGET_NAME} PRIVATE ${PCL_INCLUDE_DIRS})
link_directories(${TARGET_NAME} ${PCL_LIBRARY_DIRS})
add_definitions(${PCL_DEFINITIONS})
target_link_libraries(${TARGET_NAME} PRIVATE
  ${PCL_COMMON_LIBRARIES} 
  ${PCL_IO_LIBRARIES}
  ${PCL_FEATURES_LIBRARIES}
  ${PCL_KDTREE_LIBRARIES}
  ${PCL_ML_LIBRARIES}
  ${PCL_OCTREE_LIBRARIES}
#  ${PCL_REGISTRATION_LIBRARIES}
  ${PCL_SEARCH_LIBRARIES}
#  ${PCL_SEGMENTATION_LIBRARIES}
)
target_include_directories(${TARGET_NAME} PRIVATE "/usr/include/flann")

message("PCL_INCLUDE_DIRS=${PCL_INCLUDE_DIRS}")

#QGLViewer
target_include_directories(${TARGET_NAME} PRIVATE "./tools/QGLViewer/include")
if(WIN32)
  target_link_directories(${TARGET_NAME} PRIVATE "./tools/QGLViewer/lib/win")
  target_link_libraries(${TARGET_NAME} PRIVATE debug QGLViewerd2 optimized QGLViewer2)
elseif(UNIX)
  target_link_directories(${TARGET_NAME} PRIVATE "./tools/QGLViewer/lib/unix")
  target_link_libraries(${TARGET_NAME} PRIVATE QGLViewer-qt5)
endif(WIN32)

# GL sudo apt-get install libqglviewer-dev-qt5
if(WIN32)
  target_link_libraries(${TARGET_NAME} PRIVATE OpenGL32 GLU32)
elseif(UNIX)
  target_link_libraries(${TARGET_NAME} PRIVATE GL GLU)
endif()

# spdlog
target_include_directories(${TARGET_NAME} PRIVATE "./tools/spdlog/spdlog_1_9_2/include")
target_link_directories(${TARGET_NAME} PRIVATE "./tools/spdlog/spdlog_1_9_2/lib")
target_link_libraries(${TARGET_NAME} PRIVATE spdlog) #bpf z dl bpf

# pcie 艾利光
target_include_directories(${TARGET_NAME} PRIVATE "./tools/pcie_sdk/include")
if(WIN32)
target_link_directories(${TARGET_NAME} PRIVATE "./tools/pcie_sdk/lib/mingw32")
elseif(UNIX)
target_link_directories(${TARGET_NAME} PRIVATE "./tools/pcie_sdk/lib/linux/x86_64")
endif()
target_link_libraries(${TARGET_NAME} PRIVATE pcie_sdk_cuda pcie_sdk_stream pcie_sdk)

# pandar
target_include_directories(${TARGET_NAME} PRIVATE "./tools/pandar/include")
target_link_directories(${TARGET_NAME} PRIVATE "./tools/pandar/lib")
target_link_libraries(${TARGET_NAME} PRIVATE PandarSwiftSDK)

# pthread
target_link_libraries(${TARGET_NAME} PRIVATE -lpthread -lm -lboost_system)

# openSSL
find_package(OpenSSL REQUIRED)
target_link_libraries(${TARGET_NAME} PRIVATE OpenSSL::SSL)

#target_link_libraries(${TARGET_NAME} PRIVATE  -lopencv_imgcodecs -lopencv_core -lopencv_imgproc)

# cxkjcan
target_include_directories(${TARGET_NAME} PRIVATE "./tools/cxkj_can/include")
if(WIN32)
target_link_directories(${TARGET_NAME} PRIVATE "./tools/cxkj_can/lib")
elseif(UNIX)
target_link_directories(${TARGET_NAME} PRIVATE "./tools/cxkj_can/lib")
endif()
target_link_libraries(${TARGET_NAME} PRIVATE usb-1.0 controlcan pthread)

# ffmpeg
# target_include_directories(${TARGET_NAME} PRIVATE "/home/xy/Dependency/ffmpeg-6.0/include")
# target_link_directories(${TARGET_NAME} PRIVATE "/home/xy/Dependency/ffmpeg-6.0/lib")
target_include_directories(${TARGET_NAME} PRIVATE "/usr/local/ffmpeg/include")
target_link_directories(${TARGET_NAME} PRIVATE "/usr/local/ffmpeg/lib")
target_link_libraries(${TARGET_NAME} PRIVATE -lavcodec -lavdevice -lavfilter -lavformat -lavutil -lswresample -lswscale)
#poco
find_package(Poco REQUIRED Foundation Net NetSSL JSON)
target_link_directories(${TARGET_NAME} PRIVATE ${Poco_LIBRARY_DIRS})
target_link_libraries(${TARGET_NAME} PRIVATE ${Poco_LIBRARIES})

#OpenMp
FIND_PACKAGE( OpenMP REQUIRED)
if(OPENMP_FOUND)
    message("OPENMP FOUND")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()