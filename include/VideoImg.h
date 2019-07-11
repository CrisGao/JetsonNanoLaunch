#ifndef _VIDEO_IMG_H
#define _VIDEP_IMG_H

#include <cstdio>
#include <ctime>
#include <iostream>
#include <fstream>
#include "uart_configuration.h"
#include "Classification.h"

#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

typedef std::pair<string, float> Prediction;

void *WriteVideo_Speed(void *ptr);

void *Classify_Work(void *ptr);

Prediction GetPreScore_Max(cv::Mat Input_img);

void cleanup(void *arg);

bool Check_Road_Side(int code = 0);

class VideoImg
{
public:
    VideoImg();
    ~VideoImg();

    bool InitCamera(int capture_width, int capture_height, int display_width, int display_height, int framerate, int flip_method);
    bool startCamera();
    void saveImage();
    void delteThreadSources();
    void deleteMutexSources();
    void Input_Key(int code);

    pthread_t startThread_saveVideoSpeed();

    pthread_t startThread_classify();
    
    cv::Mat getImg;

    std::string pipeline;
    std::string sImageName;
    std::string imshowFilename;
    std::string imshowClassify = "Ready for Classification...";
    std::string imshowErrorSide = "Error:Result of clasification is RoadWay ";
    std::string imshowErrorRoad = "Error:Result of clasification is SideWalk ";

private:
    pthread_t VideoImg_thread_id;

    pthread_t Classify_thread_id;

    cv::VideoCapture *cap;

    std::string gstreamer_pipeline(int capture_width, int capture_height, int display_width, int display_height, int framerate, int flip_method)
    {
        return "nvarguscamerasrc ! video/x-raw(memory:NVMM), width=(int)" + std::to_string(capture_width) + ", height=(int)" +
               std::to_string(capture_height) + ", format=(string)NV12, framerate=(fraction)" + std::to_string(framerate) +
               "/1 ! nvvidconv flip-method=" + std::to_string(flip_method) + " ! video/x-raw, width=(int)" + std::to_string(display_width) + ", height=(int)" +
               std::to_string(display_height) + ", format=(string)BGRx ! videoconvert ! video/x-raw, format=(string)BGR ! appsink";
    }

};

#endif //_VIDEO_IMG_H
