#pragma once

#include <string>
#include "..\xrCore\xrCore.h"

#include <fstream>
#include <thread>

#ifdef XRDOWNLOADER_EXPORTS
#define XRDOWNLOADER_API __declspec(dllexport)
#define CURL_STATICLIB
#include "..\components\libcurl\include\curl\curl.h"

#else
#define XRDOWNLOADER_API __declspec(dllimport)
#pragma comment(lib,"xrDownloader.lib")
#endif

int iTotalProgress = 0;

class XRDOWNLOADER_API Xr_Downloader
{
private:
	double *PR;
	
	
private:
	std::string CorrectFilename(std::string ssss);

	static int CURL_ProgressUpdate(Xr_Downloader *D,
		double dltotal, double dlnow,
		double ultotal, double ulnow)
	{
		iTotalProgress = (dlnow / dltotal) * 100;
		return 0;
	}
public:
	void DownloadFile(std::string Url, std::string OutFile_);
	int GetProgress();
};

