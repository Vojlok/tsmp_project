#ifndef __GNUC__
#pragma once
#endif
#ifndef __TOOLS_BASE_H__
#define __TOOLS_BASE_H__

#include <string>

#ifdef _CONSOLE
#include "xr_cl_parser.h"
#endif

class tools_base {
public:
	virtual	~tools_base();

	enum source_format {
#ifdef _CONSOLE
		TOOLS_AUTO	 = 0,
#else
		TOOLS_NULL	 = 0,
#endif

		TOOLS_OGF	 = 0x001,
		TOOLS_OMF	 = 0x002,
		TOOLS_DM	 = 0x004,
		TOOLS_LEVEL	 = 0x008,
		TOOLS_OGG	 = 0x010,
		TOOLS_DDS	 = 0x020,
		TOOLS_DB	 = 0x040,
		TOOLS_FOLDER = 0x080,
		TOOLS_FANCY	 = 0x100,
		TOOLS_XRDEMO = 0x200,
	};

#ifdef _CONSOLE
	virtual void process(const cl_parser& cl) = 0;
#endif

	void check_path(const char* path, bool& status) const;
};

inline tools_base::~tools_base() {}

#endif
