// xrDownloader.cpp: определяет экспортированные функции для приложения DLL.
//

#include "stdafx.h"
#include "xrDownloader.h"

CURL *curl = NULL;

std::string Xr_Downloader::CorrectFilename(std::string ssss)
{
		std::string DPath = ssss;

		for (int i = 0; ; i++)
		{
			if (DPath[i] == '/') DPath[i] = '\\';
			if (DPath[i] == '\\')
			{
				DPath += " ";

				for (int j = DPath.size() - 1; j > i; j--)
				{
					DPath[j] = DPath[j - 1];
				}

				DPath[i + 1] = '\\';
				i++;
			}

			if (i == (DPath.size() - 1)) break;

		}
		Msg("was %s become %s", ssss.c_str(), DPath.c_str());

		return DPath;
	}

void Xr_Downloader::DownloadFile(std::string Url, std::string OutFile_)
{
		Msg("ThD: DownloadFile Called");
	curl = curl_easy_init();

		iTotalProgress = 0;

		Msg("ThD: CorrectFile Called");
		std::string OutFile = CorrectFilename(OutFile_);
		Msg("ThD: CorrectFile Finished");

		FILE *fp;
		CURLcode res;

		if (curl)
		{
			Msg("ThD: Curl initialization ok");

			fp = fopen(OutFile.c_str(), "wb");
			curl_easy_setopt(curl, CURLOPT_URL, Url.c_str());
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL);

			curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);
			curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, CURL_ProgressUpdate);
			curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, &PR);

			curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
			res = curl_easy_perform(curl);
			fclose(fp);

			Msg("task completed!");
			curl_easy_cleanup(curl);
		}
	}

int Xr_Downloader::GetProgress()
{
	return iTotalProgress;
}