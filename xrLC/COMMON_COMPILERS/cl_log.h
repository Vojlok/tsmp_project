#pragma	once

#include <windows.h>

void clMsg			(const char *format, ...);
void Status			(const u32 uItemsCount, const char *format, ...);
void Progress		(const u32 uProgress);
void Progress		(const float F);
void Phase			(const char *phase_name);

void logThread	    (void *dummy);
void logCallback	(LPCSTR c);
