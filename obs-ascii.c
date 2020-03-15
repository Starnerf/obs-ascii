#include <obs-module.h>

OBS_DECLARE_MODULE()
//OBS_MODULE_USE_DEFAULT_LOCALE("obs-ascii", "en-US")
MODULE_EXPORT const char* obs_module_description(void)
{
	return "OBS ASCII plugin";
}

//extern struct obs_source_info caca_filter;
extern struct obs_source_info ascii_filter;

bool obs_module_load(void)
{
	//obs_register_source(&caca_filter);
	obs_register_source(&ascii_filter);
	return true;
}
