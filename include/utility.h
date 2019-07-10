#ifndef _UTILITY_H
#define _UTILITY_H

#include <cstdio>
#include <ctime>
#include <iostream>
#include <fstream>
#include <stdio.h>

std::string currentTime()
{
	time_t rawtime;
	struct tm *ptminfo;
	char buf[128]={0};
	time(&rawtime);
	ptminfo = localtime(&rawtime);
	
	std::stringstream ss;
	std::string str;
	ss<<(ptminfo->tm_year+1900)<<(ptminfo->tm_mon+1)<<ptminfo->tm_mday<<ptminfo->tm_hour<<ptminfo->tm_min<<ptminfo->tm_sec;       
	ss>>str;
	return str;
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
