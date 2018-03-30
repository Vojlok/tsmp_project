#ifndef __GNUC__
#pragma once
#endif
#ifndef __OGG_TOOLS_H__
#define __OGG_TOOLS_H__

#include "tools_base.h"

class ogg_tools: public tools_base {
public:
#ifdef _CONSOLE
	virtual void process(const cl_parser& cl);
#else
	void process(const char* path);
#endif

private:
	bool check_paths() const;
	void process_folder(const std::string& path = "");
	void process_file(const std::string& path);

	std::string	m_sounds;
};

#endif
