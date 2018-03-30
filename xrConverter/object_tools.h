#ifndef __GNUC__
#pragma once
#endif
#ifndef __OBJECT_TOOLS_H__
#define __OBJECT_TOOLS_H__

#include <string>
#include "tools_base.h"
#include "batch_helper.h"

namespace xray_re {
	class xr_object;
};

class object_tools: public tools_base, public batch_helper {
public:
	enum target_format {
		TARGET_DEFAULT	= 0,

		TARGET_INFO		= 0x01,
		TARGET_SKL		= 0x02,
		TARGET_SKLS		= 0x04,
		TARGET_BONES	= 0x08,
		TARGET_OBJECT	= 0x10,

		TARGET_ERROR	= 0x1000,
	};

#ifdef _CONSOLE
	virtual void process(const cl_parser& cl) = 0;
#endif

protected:
	void save_object(xray_re::xr_object& object, const char* source) const;

#ifdef _CONSOLE
	target_format get_target_format(const cl_parser& cl) const;
#else
	target_format get_target_format(const char* format) const;
#endif
};

#endif
