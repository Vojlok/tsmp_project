#include "stdafx.h"
#include "build.h"
#include "common_compilers\xrThread.h"

#include <thread>
#include <mutex>

class CMULight : public CThread
{
	u32			low;
	u32			high;
public:
	CMULight(u32 ID, u32 _low, u32 _high) : CThread(ID) { thMessages = TRUE; low = _low; high = _high; }

	virtual void	Execute()
	{
		// Priority
		SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_BELOW_NORMAL);
		if (b_highest_priority) SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
		Sleep(0);

		// Light references
		for (u32 m = low; m<high; m++)
		{
			pBuild->mu_refs[m]->calc_lighting();
			thProgress = (float(m - low) / float(high - low));
		}
	}
};

void	CBuild::xrPhase_MU_light()
{

	/*
		// Light models
		for (u32 m = 0; m < pBuild->mu_models.size(); m++)
		{
			pBuild->mu_models[m]->calc_materials();
			pBuild->mu_models[m]->calc_lighting();
		}
	*/

	CTimer	start_mu1;
	start_mu1.Start();

	auto ThTh = [](const int from, const int to)
	{
		for (u32 m = from; m < to; m++)
		{
			pBuild->mu_models[m]->calc_materials();
			pBuild->mu_models[m]->calc_lighting();
		}
	};
	clMsg("%i size", mu_models.size());

	int half = mu_models.size()/2;
//	std::thread thread_1(ThTh,0,half);
//	std::thread thread_2(ThTh,half, mu_models.size());
//	thread_1.join();
//	thread_2.join();

	std::thread thread_1(ThTh, 0, mu_models.size());
	thread_1.join();

	clMsg("%f seconds", start_mu1.GetElapsed_sec());


		// Light references
		u32	count, stride, threads_count;
		count = pBuild->mu_refs.size();

		if (count)
		{
			Gen_thread_intervals(8, count, threads_count, stride);

			CThreadManager mu_sec;

			CTimer	start_mu;
			start_mu.Start();

			for (u32 thID = 0; thID < threads_count; thID++)
				mu_sec.start(xr_new<CMULight>(thID, thID*stride, ((thID == (threads_count - 1)) ? count : (thID*stride + stride))));
			mu_sec.wait(500);

			clMsg("%f seconds", start_mu.GetElapsed_sec());
		}
	}
