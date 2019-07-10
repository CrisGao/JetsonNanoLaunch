#include "uart_configuration.h"
#include "VideoImg.h"


//#include <Classification.h>

#include <iostream>
#include <string.h>
#include <stdint.h>
#include<stdio.h>      /*标准输入输出定义*/
#include<stdlib.h>     /*标准函数库定义*/
#include<unistd.h>     /*Unix 标准函数定义*/
#include<sys/types.h> 
#include<sys/stat.h>  

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

#if 0
string model_file   = "../data/deploy.prototxt";
string trained_file = "../data/2caffe_train_iter_70000.caffemodel";
string mean_file    = "../data/mean.binaryproto";
string label_file   = "../data/label.txt";
#endif

int main()
{

 

      pthread_t uart_id,videoImg_id, Classify_id;

      jetsonSerial *JetsonUart = new jetsonSerial();
      VideoImg *JetsonVideo = new VideoImg();//capture_w,capture_h,display_w,display_h,framerate,flip_method
     // ClassifyThread *JetsonClassify =  new ClassifyThread(); 
      //JetsonVideo->Init_classification(model_file,trained_file,mean_file,label_file);//初始化分类Classify();

      char *OpenFile = "/dev/ttyUSB0";
      JetsonUart->Transceriver_UART_init(OpenFile,9600,0,8,1,'N');

      uart_id = JetsonUart->startThread();
      
      if(!JetsonVideo->InitCamera(3280,2464,1280,720,20,2))
	{
	return -1;
	}

      while(true)
      {
            if(!JetsonVideo->startCamera())
            {
		break;
            }
            int keycode = cv::waitKey(30) & 0xff;
	    if(keycode == 115)//key is s,function for save image
	    {
		  std::cout << "---------- SaveCurrentImage ----------" << std::endl;
                  JetsonVideo->saveImage();
            }
            if(keycode == 118)//key is v,function for save video
            {
                  if(!if_start_save)
                  {
                        ifsave = true;
                        
                  }
                  else
                  {
                  if(!pthread_cancel(videoImg_id))
		    {
			if_start_save = false;
			std::cout<<"success exit the videoImg_thread"<<std::endl;

		     }
			//pthread_join(videoImg_id,NULL);
			JetsonVideo->delteThreadSources();	
                        
                  }

            }
            if(ifsave)
            {
		  std::cout << "---------- SaveVideoSpeed ----------" << std::endl;
                  videoImg_id = JetsonVideo->startThread_saveVideoSpeed();
                  if_start_save = true;
                  ifsave = false;
            }

            if(keycode == 103)//key is g,function for classification
	    {
		
		if(!if_start_classify)
		{
			ifclassify = true;
		}
		else
		{
			if(!pthread_cancel(Classify_id))
			{
				if_start_classify = false;
				std::cout<<"success exit the classify_thread"<<std::endl;
			}
			//pthread_join(Classify_id,NULL);
			JetsonVideo->deleteMutexSources();
		}

		
	    }
	    if(ifclassify)
		{
			std::cout << "---------- Prediction ----------" << std::endl;
			Classify_id = JetsonVideo->startThread_classify();
			
			if_start_classify = true;
			ifclassify = false;
		}
	    if(keycode == 114)//key is r,function for checking whether current image is a roadway,if not save the wrong image
		{
			JetsonVideo->Input_Key(114);
		}
if(keycode == 121)
{
	JetsonVideo->Input_Key(121);
}
	    if(keycode == 27)//key is esc,function for exit the program
            {
 	      cv::destroyAllWindows();
	      exit(0);
              break;

            }
     
     
     }
   
  pthread_join(uart_id,NULL);
             pthread_exit(NULL);
              return 0;


           
}

