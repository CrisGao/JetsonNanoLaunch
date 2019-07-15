#ifndef _UTILITY_H
#define _UTILITY_H

#include <cstdio>
#include <ctime>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <string>

std::string currentTimeStr = "0";
int record_nums = 0;

std::string single2double(int time)
{
	std::stringstream stime;
	std::string str_time;
	if(time<10)
	{
		stime<<"0"<<time;
	}
	else
	stime<<time;
	stime>>str_time;
	return str_time;
}

std::string currentTime()
{
	time_t rawtime;
	struct tm *ptminfo;
	struct timeval* ptusecInfo;
	char buf[128]={0};
	time(&rawtime);
	ptminfo = localtime(&rawtime);
	
	std::stringstream ss;
	std::string str;
	ss<<(ptminfo->tm_year+1900)<<single2double((ptminfo->tm_mon+1))<<single2double(ptminfo->tm_mday)<<single2double(ptminfo->tm_hour)<<single2double(ptminfo->tm_min)<<single2double(ptminfo->tm_sec);
	//ss<<(ptminfo->tm_year+1900)<<"/"<<(ptminfo->tm_mon+1)<<"/"<<ptminfo->tm_mday<<"-"<<ptminfo->tm_hour<<":"<<ptminfo->tm_min<<":"<<ptminfo->tm_sec;       
	ss>>str;
	return str;
}

std::string currentImageTime(std::string time)//will change the saved filename during one second
{
	
        std::cout<<"record:"<<record_nums<<std::endl;
	std::string ReturnTime;
	std::stringstream strs;
	if(currentTimeStr.compare(time) == 0)
	{
		record_nums++;
	}
	else
	{
		record_nums = 0;

	}	
	strs<<time<<"-"<<record_nums;
	currentTimeStr = time;
	strs>>ReturnTime;
	return ReturnTime;

	
}

std::string doubleTostring(const double &dbNum)
{
	char *chCode;
	chCode = new(std::nothrow)char[20];
	sprintf(chCode,"%.2lf",dbNum);
	std::string strCode(chCode);
	delete []chCode;
	return strCode; 
}

#endif
