#include "object_tools.h"
#include "xr_object.h"
#include "xr_object_format.h"
#include "xr_file_system.h"
#include "xr_string_utils.h"

using namespace xray_re;

#if 0
void object_tools::process(const cl_parser& cl)
{
	xr_object* object = new xr_object();
	if (object->load_object(source))
		;//object->save_object("GGGGG");
	else
		msg("can't load %s", source.c_str());
	delete object;
}
#endif

void object_tools::save_object(xr_object& object, const char* source) const
{
	std::string target;
	make_target_name(target, source, ".object");
	object.to_object();
	if (!object.save_object(target.c_str()))
		msg("can't save object in %s", target.c_str());
}

#ifdef _CONSOLE
object_tools::target_format object_tools::get_target_format(const cl_parser& cl) const
{
	unsigned format = 0;
	if (cl.exist("-info"))
		format |= TARGET_INFO;
	if (cl.exist("-skl"))
		format |= TARGET_SKL;
	if (cl.exist("-skls"))
		format |= TARGET_SKLS;
	if (cl.exist("-bones"))
		format |= TARGET_BONES;
	if (cl.exist("-object"))
		format |= TARGET_OBJECT;
	return (format & (format - 1)) == 0 ? static_cast<target_format>(format) : TARGET_ERROR;
}
#else
object_tools::target_format object_tools::get_target_format(const char* format) const 
{
	if (xr_stricmp(format, "info") == 0)
		return TARGET_INFO;
	else if (xr_stricmp(format, "skl") == 0)
		return TARGET_SKL;
	else if (xr_stricmp(format, "skls") == 0)
		return TARGET_SKLS;
	else if (xr_stricmp(format, "bones") == 0)
		return TARGET_BONES;

	return TARGET_OBJECT;
}
#endif // _CONSOLE