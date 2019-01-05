#include "stdafx.h"
#include "TSMP_Timers.h"
#include "level.h"
#include "../XR_IOConsole.h"

int	TSMP_Timer1Interval = 15;
int TSMP_Timer2Interval = 15;
int	TSMP_Timer3Interval = 15;
int	TSMP_Timer1Enabled = 0;
int	TSMP_Timer2Enabled = 0;
int	TSMP_Timer3Enabled = 0;

u32	uTimer1StartTime = 0;
u32	uTimer2StartTime = 0;
u32	uTimer3StartTime = 0;

void TimerThread(void*)
{
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_BELOW_NORMAL);

	while (true)
	{
		u32 uCurTime = Level().timeServer();

		if (TSMP_Timer1Enabled)
		{
			if (uTimer1StartTime + u32(TSMP_Timer1Interval * 60000) < uCurTime)
			{
				Console->Execute("cfg_load tsmp_timer1_commands");
				uTimer1StartTime = uCurTime;
			}
		}

		if (TSMP_Timer2Enabled)
		{
			if (uTimer2StartTime + u32(TSMP_Timer2Interval * 60000) < uCurTime)
			{
				Console->Execute("cfg_load tsmp_timer2_commands");
				uTimer2StartTime = uCurTime;
			}
		}

		if (TSMP_Timer3Enabled)
		{
			if (uTimer3StartTime + u32(TSMP_Timer3Interval * 60000) < uCurTime)
			{
				Console->Execute("cfg_load tsmp_timer3_commands");
				uTimer3StartTime = uCurTime;
			}
		}

		Sleep(30000);
	}
}

void OnTimersStart()
{
	if (bIsRunning) return;
	bIsRunning = true;

	thread_spawn(TimerThread, "TimerThread", 0, 0);
}

