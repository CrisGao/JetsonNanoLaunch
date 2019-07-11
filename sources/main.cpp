#include "uart_configuration.h"
#include "VideoImg.h"

#include <iostream>
#include <string.h>
#include <stdint.h>
#include <stdio.h>  /*标准输入输出定义*/
#include <stdlib.h> /*标准函数库定义*/
#include <unistd.h> /*Unix 标准函数定义*/
#include <sys/types.h>
#include <sys/stat.h>

#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <fstream>

extern cv::Mat img;

bool ifsave = false;
bool if_start_save = false;

bool ifclassify = false;
bool if_start_classify = false;

bool ifEXIT = false;

enum InputKey
{
	key_S = 115,
	key_V = 118,
	key_G = 103,
	key_R = 114,
	key_Y = 121,
	key_ESC = 27
};

int main()
{
/*****************************usage note********************/
std::cout << "usage is:" << std::endl
					  << "V--save video" << std::endl
					  << "S--save current image" << std::endl
					  << "G--classify the image" << std::endl
					  << "R--when ride on roadway,then save error image" << std::endl
					  << "Y--when ride on sidewalk,then save error image"<< std::endl;
/*****************************END********************/	
	pthread_t uart_id, videoImg_id, Classify_id;

	jetsonSerial *JetsonUart = new jetsonSerial();
	VideoImg *JetsonVideo = new VideoImg(); 

	char *OpenFile = "/dev/ttyUSB0";
	JetsonUart->Transceriver_UART_init(OpenFile, 9600, 0, 8, 1, 'N');

	uart_id = JetsonUart->startThread();

	if (!JetsonVideo->InitCamera(3280, 2464, 1280, 720, 20, 2))
	{
		return -1;
	}

	while (true)
	{
		if (!JetsonVideo->startCamera())
		{
			break;
		}

		int keycode = cv::waitKey(30) & 0xff;

		switch (keycode)
		{
		case key_S:
			std::cout << "---------- SaveCurrentImage ----------" << std::endl;
			JetsonVideo->saveImage();
			break;

		case key_V:
			if (!if_start_save)
			{
				ifsave = true;
				
			}
			else
			{
				if (!pthread_cancel(videoImg_id))
				{
					if_start_save = false;
					std::cout << "success exit the videoImg_thread" << std::endl;
				}
				pthread_join(videoImg_id,NULL);
				if_start_save = false;
				std::cout << "success exit the videoImg_thread" << std::endl;
				JetsonVideo->delteThreadSources();
			}
			break;

		case key_G: 
			if (!if_start_classify)
			{
				ifclassify = true;
				
			}
			else
			{
				if (!pthread_cancel(Classify_id))
				{
					if_start_classify = false;
					std::cout << "success exit the classify_thread" << std::endl;
				}
				pthread_join(Classify_id,NULL);
				JetsonVideo->deleteMutexSources();
			}
			break;

		case key_R:
			JetsonVideo->Input_Key(114);
			break;

		case key_Y:
			JetsonVideo->Input_Key(121);
			break;

		case key_ESC:
			cv::destroyAllWindows();
			exit(0);
			ifEXIT = true;
			break;

		default:
			break;
		}

		if (ifsave)
		{
			std::cout << "---------- SaveVideoSpeed ----------" << std::endl;
			videoImg_id = JetsonVideo->startThread_saveVideoSpeed();
			if_start_save = true;
			ifsave = false;
		}
		if (ifclassify)
		{
			std::cout << "---------- Prediction ----------" << std::endl;
			Classify_id = JetsonVideo->startThread_classify();

			if_start_classify = true;
			ifclassify = false;
		}
		if (ifEXIT)
			break;
	}

	pthread_join(uart_id, NULL);


	return 0;
}
