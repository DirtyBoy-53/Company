INCLUDEPATH += $$PWD
DEPENDPATH  += $$PWD


win32{

}

unix{
    INCLUDEPATH += /usr/include/opencv4 \
                    /usr/include/opencv4/opencv2 \

    LIBS        += -L//usr/lib/x86_64-linux-gnu \
                    -lopencv_core \
                    -lopencv_imgcodecs \
                    -lopencv_highgui \
                    -lopencv_imgproc

    # INCLUDEPATH +=  $$/opencv-3.4.16/opencv/header \
    #                 $$PWD/linux/opencv-3.4.16/opencv/header/opencv \
    #                 $$PWD/linux/opencv-3.4.16/opencv/header/opencv2 \

    # LIBS += -L$$PWD/linux/opencv-3.4.16/opencv/lib \
    #                 -lopencv_core \
    #                 -lopencv_imgcodecs \
    #                 -lopencv_highgui \
    #                 -lopencv_imgproc

}
