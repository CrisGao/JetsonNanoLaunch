#include "VideoImg.h"
#include <stdint.h>
#include<stdio.h>      /*标准输入输出定义*/
#include<stdlib.h>     /*标准函数库定义*/
#include<unistd.h>     /*Unix 标准函数定义*/
#include<sys/types.h> 
#include<sys/stat.h>  
#include <string>

#include "utility.h"


extern double VehicleSpeed;
//extern mutex1;
cv::Mat img;

bool ifshow = false; 
bool ifshowClassify = false;

bool ifCheck = false;
int isRoadSide;

bool ifshowErrorSide = false;
bool ifshowErrorRoad = false;

pthread_mutex_t Img_mutex;
Classifier* newClassf;


VideoImg::VideoImg()
{
	

}
VideoImg::~VideoImg()
{
  	cap->release();
	pthread_exit(NULL);
        //pthread_exit(&VideoImg_thread_id);
    	//pthread_exit(&Classify_thread_id);
    	pthread_mutex_destroy(&Img_mutex);


}

bool VideoImg::InitCamera(int capture_width,int capture_height,int display_width,int display_height,int framerate,int flip_method)
{
	//pthread_mutexattr_t mattr;
	//pthread_mutexattr_init(&mattr);
	//pthread_mutexattr_setpshared(&mattr,PTHREAD_PROCESS_SHARED);
        //pthread_mutex_init(Img_mutex,&mattr);
	pthread_mutex_init(&Img_mutex,NULL);
	
        pipeline = gstreamer_pipeline(capture_width,capture_height,display_width,display_height,framerate,flip_method);
        cap = new cv::VideoCapture(pipeline, cv::CAP_GSTREAMER);
        //cap.open("test.avi");

	if(!cap->isOpened())
	{

	std::cout<<"Failed to open camera."<<std::endl;
           return false;
	
	}
        cv::namedWindow("CSI Camera", cv::WINDOW_NORMAL);
        cv::setWindowProperty("CSI Camera",cv::WND_PROP_FULLSCREEN,cv::WINDOW_FULLSCREEN);
std::cout<<"get avi"<<std::endl;
        return true;
    
}
bool VideoImg::startCamera()
{
	
        imshowFilename = "CurrentSpeed is:"+doubleTostring(VehicleSpeed)+"km/h";

	pthread_mutex_lock(&Img_mutex);
    	if (!cap->read(img)) 
        {
            std::cout<<"Capture read error"<<std::endl;
            return false;
        }
/**********下面if判断为测试显示用********************/	
        if(ifshow)//imshow current speed
	{
        
          cv::putText(img,imshowFilename,cv::Point(50,60),cv::FONT_HERSHEY_SIMPLEX,1,cv::Scalar(255,23,0),4,8);
	}
	if(ifshowClassify)//initivate Classify class
	{
		
 	  cv::putText(img,imshowClassify,cv::Point(50,60),cv::FONT_HERSHEY_SIMPLEX,1,cv::Scalar(255,23,0),4,8);
	}
	if(ifshowErrorSide)//when ride on sidewalk but capture roadway
	{
 	  cv::putText(img,imshowErrorSide,cv::Point(50,60),cv::FONT_HERSHEY_SIMPLEX,1,cv::Scalar(0,0,255),4,8);
	}
	if(ifshowErrorRoad)//when ride on roadway but capture sidewalk
	{
 	  cv::putText(img,imshowErrorRoad,cv::Point(50,60),cv::FONT_HERSHEY_SIMPLEX,1,cv::Scalar(0,0,255),4,8);
	}
/****************************************************/
        cv::imshow("CSI Camera",img);
	pthread_mutex_unlock(&Img_mutex);
        return true;
     
}
void cleanup(void* arg)
{
  pthread_mutex_unlock(&Img_mutex);
}
void cleanup1(void* arg)
{
  pthread_mutex_unlock(&Img_mutex);
}
void* WriteVideo_Speed(void* ptr)
{  
     //pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    // pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED,NULL);
    int i=0 ; 
    ifshow = true;
    std::string imageFileName;

    std::string Videoname,VideoSpeedFilename;
    Videoname="../video/"+currentTime()+".avi";
    std::cout<<"Videoname:"<<Videoname<<std::endl;
    VideoSpeedFilename = "../Params/"+currentTime()+".yml";
    std::cout<<"Save Speed YML in :"<<VideoSpeedFilename<<std::endl;
    cv::FileStorage fs(VideoSpeedFilename, cv::FileStorage::WRITE);
   cv::VideoWriter writer =cv::VideoWriter(Videoname,CV_FOURCC('M','J','P','G'),20,cv::Size(1280,720),1);
    while(1)
       {
	   pthread_cleanup_push(cleanup,NULL);  
	   pthread_mutex_lock(&Img_mutex);
            writer.write(img); 
	   pthread_mutex_unlock(&Img_mutex);
	   pthread_cleanup_pop(0);   
            if (fs.isOpened())
            {
                imageFileName = "Frame"+std::to_string(i);
		std::cout<<"save sequences:"<<imageFileName<<",speed:"<<VehicleSpeed<<std::endl;
                fs << imageFileName << VehicleSpeed ;
                
            }
            else
            {
                std::cout << "Error: can not save the VehicleSpeedParams!!!!!" << std::endl;
            }
           
            i++;

            //usleep(50000);//好像可以去掉，因为write会有延时

       }
      
      fs.release();
      writer.release();
}

void VideoImg::delteThreadSources()
{
        
        //pthread_mutex_destroy(&Img_mutex);
       ifshow = false;
       std::cout<<"ifshow:false"<<std::endl;

}

void VideoImg::saveImage()
{
		//pthread_mutex_lock(&Img_mutex);
		sImageName = "../saveImg/"+currentTime()+".jpg";
		std::cout<<"currentImage is:"<<sImageName<<std::endl;
		cv::imwrite(sImageName,img);
		///pthread_mutex_unlock(&Img_mutex);
}

pthread_t VideoImg::startThread_saveVideoSpeed()
{
        
         if(pthread_create(&VideoImg_thread_id,NULL,WriteVideo_Speed,NULL))
        {
            std::cout<<"VideoImg_thread create error!"<<std::endl;
            return -1;
        }
        else
        {
            std::cout<<"VideoImg_thread create success!"<<std::endl;
            return VideoImg_thread_id;
        }
        
    
}

/******classification work***************************/
void VideoImg::Input_Key(int code)
{
    ifCheck = Check_Road_Side(code);
}
bool Check_Road_Side(int code)
{
	if(code == 114)
	{
		isRoadSide = 114;
		return true;
	}
	else if(code == 121)
	{
		isRoadSide = 121;
		return true;
	}
	else
	{
		return false;
	}	

}

void* Classify_Work(void* ptr)
{
ifshowClassify = true;
string model_file   = "../data/deploy.prototxt";
string trained_file = "../data/2caffe_train_iter_70000.caffemodel";
string mean_file    = "../data/mean.binaryproto";
string label_file   = "../data/label.txt";
      newClassf = new Classifier(model_file,trained_file,mean_file,label_file);//为防止new多个对象，取消线程的时候delete对象。另一方面可以把类写成单例模式，但是一直报私有成员无法访问错误。
std::cout<<"had getinstance"<<std::endl;
ifshowClassify = false;
	while(1)
	{
		cv::Mat input_image; 
		pthread_cleanup_push(cleanup1,NULL); 
		pthread_mutex_lock(&Img_mutex);
		input_image = img.clone();
		pthread_mutex_unlock(&Img_mutex);
		pthread_cleanup_pop(0);
		Prediction maxS = GetPreScore_Max(input_image);	
		
		std::cout << std::fixed << std::setprecision(4) << maxS.second << " - \"" << maxS.first << "\"" << std::endl;
       
		if(maxS.first.compare("0 RoadWay")==0)
		{
			system("play 6601.wav");
		}
		
		if(ifCheck)
		{
		if(isRoadSide == 114)	//在非机动车道行驶，保存检测结果为机动车道的图像，控制按钮为"r"
		{
			ifshowErrorRoad = false;
			if(maxS.first.compare("0 RoadWay")==0)
			{
			ifshowErrorSide = true;
			
 		std::string Road_errImage = "../ErrorImg/Side_Road_"+currentTime()+".jpg";
		std::cout<<"BadImage is:"<<Road_errImage<<std::endl;
		cv::imwrite(Road_errImage,img);
			}
		}
		else  //在机动车道行驶，保存检测结果为非机动车道的图像，控制按钮为"y"
		{
			ifshowErrorSide = false;
			
              		if(maxS.first.compare("1 RoadSid")==0)
			{
			
			ifshowErrorRoad = true;
 		std::string Side_errImage = "../ErrorImg/Road_Side_"+currentTime()+".jpg";
		std::cout<<"BadImage is:"<<Side_errImage<<std::endl;
		cv::imwrite(Side_errImage,img);
			}
			
		}	
		}
		
		
		usleep(50000);
	}
	
	
}

Prediction GetPreScore_Max(cv::Mat Input_img)
{

	cv::Mat Input_image = Input_img;
	CHECK(!Input_image.empty())<<"Unabel to decode image"<<std::endl;
	std::vector<Prediction>predictions =newClassf->Classify(Input_image); 
       vector<double>Pscore;
	for(size_t i = 0;i < predictions.size();++i)
	{
		Prediction p = predictions[i];
		Pscore.push_back(p.second);
	}
	auto PreScore = max_element(Pscore.begin(),Pscore.end());
	for(size_t i = 0;i < predictions.size();++i)
	{
		Prediction PreS  = predictions[i];
		if(*PreScore == PreS.second)
		{
			//std::cout<<std::fixed<<std::setprecision(4)<<PreS.second<<" - \""<<PreS.first<< "\""<<std::endl;
			return PreS;
		}
	}
 
}

pthread_t VideoImg::startThread_classify()
{      
	if(pthread_create(&Classify_thread_id,NULL,Classify_Work,NULL))
        {
            std::cout<<"Classify_thread create error!"<<std::endl;
            return -1;
        }
        else
        {
            std::cout<<"Classify_thread create success!"<<std::endl;
            return Classify_thread_id;
        }

}
void VideoImg::Init_classification(string model_file,string trained_file,string mean_file,string label_file)
{

	//newClassf = new Classifier(model_file,trained_file,mean_file,label_file);
}
void VideoImg::deleteMutexSources()
{
	ifCheck = false;
	ifshowErrorSide = false;
	ifshowErrorRoad = false;
	delete newClassf;
	newClassf = NULL;//释放申请的对象内存和指向
        pthread_mutex_destroy(&Img_mutex);
}



