#include <obs-module.h>
#include <graphics/vec2.h>

struct ascii_filter_data {
	obs_source_t                   *context;

	gs_effect_t                    *effect;
	gs_eparam_t		       *param_char_spacing;
	gs_eparam_t		       *param_char_size;
	gs_eparam_t		       *param_color_mode;
	gs_eparam_t		       *param_invert_brightness;
	gs_eparam_t		       *param_dithering;
	gs_eparam_t		       *param_dither_intensity;
	gs_eparam_t		       *param_width;
	gs_eparam_t		       *param_height;
	gs_eparam_t		       *param_bg_color;
	gs_eparam_t		       *param_calc_bg_color;
	gs_eparam_t		       *param_char_color_scale;
	gs_eparam_t		       *param_bg_color_scale;

	int			       char_spacing;
	bool			       char_size;
	int			       color_mode;
	bool			       invert_brightness;
	bool			       dithering;
	float			       dither_intensity;
	int                            width;
	int                            height;
	float			       font_size;
	struct vec3		       bg_color;
	bool			       calc_bg_color;
	float			       char_color_scale;
	float			       bg_color_scale;
};

static const char *ascii_filter_get_name(void *unused)
{
	UNUSED_PARAMETER(unused);
	return "ASCII Filter";
}

static void *ascii_filter_create(obs_data_t *settings, obs_source_t *context)
{
	struct ascii_filter_data *filter = bzalloc(sizeof(*filter));
	char *effect_path = obs_module_file("ASCII_obs.effect");
	char err_str[32][128];

	filter->context = context;

	obs_enter_graphics();
	filter->effect = gs_effect_create_from_file(effect_path, err_str);
	obs_leave_graphics();

	bfree(effect_path);

	if (!filter->effect) {
		bfree(filter);
		return NULL;
	}

	filter->param_char_spacing = gs_effect_get_param_by_name(filter->effect, "Ascii_spacing");
	filter->param_char_size = gs_effect_get_param_by_name(filter->effect, "Ascii_font");
	filter->param_color_mode = gs_effect_get_param_by_name(filter->effect, "Ascii_font_color_mode");
	filter->param_invert_brightness = gs_effect_get_param_by_name(filter->effect, "Ascii_invert_brightness");
	filter->param_dithering = gs_effect_get_param_by_name(filter->effect, "Ascii_dithering");
	filter->param_dither_intensity = gs_effect_get_param_by_name(filter->effect, "Ascii_dithering_intensity");
	filter->param_width = gs_effect_get_param_by_name(filter->effect, "screen_width");
	filter->param_height = gs_effect_get_param_by_name(filter->effect, "screen_height");
	filter->param_bg_color = gs_effect_get_param_by_name(filter->effect, "Ascii_background_color");
	filter->param_calc_bg_color = gs_effect_get_param_by_name(filter->effect, "calc_bg_color");
	filter->param_char_color_scale = gs_effect_get_param_by_name(filter->effect, "char_color_scale");
	filter->param_bg_color_scale = gs_effect_get_param_by_name(filter->effect, "bg_color_scale");

	filter->font_size = 1.0;

	obs_source_update(context, settings);
	return filter;
}

static void ascii_filter_destroy(void *data)
{
	struct ascii_filter_data *filter = data;

	obs_enter_graphics();
	gs_effect_destroy(filter->effect);
	obs_leave_graphics();

	bfree(filter);
}

static void ascii_filter_update(void *data, obs_data_t *settings)
{
	struct ascii_filter_data *filter = data;

	filter->char_spacing = (int)obs_data_get_int(settings, "Ascii_spacing");
	filter->char_size = obs_data_get_bool(settings, "Ascii_font");
	filter->color_mode = (int)obs_data_get_int(settings, "Ascii_font_color_mode");
	filter->invert_brightness = obs_data_get_bool(settings, "Ascii_invert_brightness");
	filter->dithering = obs_data_get_bool(settings, "Ascii_dithering");
	filter->dither_intensity = (float)obs_data_get_double(settings, "Ascii_dithering_intensity");
	filter->font_size = (float)obs_data_get_double(settings, "font_size");
	filter->bg_color.x = (float)obs_data_get_double(settings, "bg_color_r");
	filter->bg_color.y = (float)obs_data_get_double(settings, "bg_color_g");
	filter->bg_color.z = (float)obs_data_get_double(settings, "bg_color_b");
	filter->calc_bg_color = obs_data_get_bool(settings, "calc_bg_color");
	filter->char_color_scale = (float)obs_data_get_double(settings, "char_color_scale");
	filter->bg_color_scale = (float)obs_data_get_double(settings, "bg_color_scale");
}

static obs_properties_t *ascii_filter_properties(void *data)
{
	obs_properties_t *props = obs_properties_create();

	obs_properties_add_int(props, "Ascii_spacing", "Character spacing",
		0, 5, 1);
	obs_properties_add_bool(props, "Ascii_font", "Character size (off = 3x5, on = 5x5)");
	obs_properties_add_int(props, "Ascii_font_color_mode", "Color Mode",
		0, 2, 1);
	obs_properties_add_bool(props, "Ascii_invert_brightness", "Invert brightness");
	obs_properties_add_bool(props, "Ascii_dithering", "Dithering");
	obs_properties_add_float(props, "Ascii_dithering_intensity", "Dithering Intensity",
		0.0, 4.0, 0.1);
	obs_properties_add_float(props, "font_size", "Font size",
		1.0, 20.0, 0.1);
	obs_properties_add_float(props, "bg_color_r", "Background Color (red)",
		0.0, 1.0, 0.01);
	obs_properties_add_float(props, "bg_color_g", "Background Color (green)",
		0.0, 1.0, 0.01);
	obs_properties_add_float(props, "bg_color_b", "Background Color (blue)",
		0.0, 1.0, 0.01);
	obs_properties_add_bool(props, "calc_bg_color", "Calculate background color");
	obs_properties_add_float(props, "char_color_scale", "Character brightness scaling",
		0.00, 2.0, 0.01);
	obs_properties_add_float(props, "bg_color_scale", "Background brightness scaling",
		0.00, 2.0, 0.01);

	UNUSED_PARAMETER(data);
	return props;
}

static void ascii_filter_defaults(obs_data_t *settings)
{
	obs_data_set_default_int(settings, "Ascii_spacing", 1);
	obs_data_set_default_bool(settings, "Ascii_font", true);
	obs_data_set_default_int(settings, "Ascii_font_color_mode", 2);
	obs_data_set_default_bool(settings, "Ascii_invert_brightness", false);
	obs_data_set_default_bool(settings, "Ascii_dithering", false);
	obs_data_set_default_double(settings, "Ascii_dithering_intensity", 1.0);
	obs_data_set_default_double(settings, "font_size", 1.0);
	obs_data_set_default_double(settings, "bg_color_r", 0.0);
	obs_data_set_default_double(settings, "bg_color_g", 0.0);
	obs_data_set_default_double(settings, "bg_color_b", 0.0);
	obs_data_set_default_bool(settings, "calc_bg_color", false);
	obs_data_set_default_double(settings, "char_color_scale", 1.0);
	obs_data_set_default_double(settings, "bg_color_scale", 1.0);
}

static void calc_ascii_dimensions(struct ascii_filter_data *filter)
{
	obs_source_t *target = obs_filter_get_target(filter->context);
	uint32_t width;
	uint32_t height;

	if (!target) {
		width = 0;
		height = 0;
		return;
	} else {
		width = obs_source_get_base_width(target);
		height = obs_source_get_base_height(target);
	}
}

static void ascii_filter_tick(void *data, float seconds)
{
	struct ascii_filter_data *filter = data;
	calc_ascii_dimensions(data);
	UNUSED_PARAMETER(seconds);
}

static void ascii_filter_render(void *data, gs_effect_t *effect)
{
	struct ascii_filter_data *filter = data;

	if (!obs_source_process_filter_begin(filter->context, GS_RGBA,
				OBS_ALLOW_DIRECT_RENDERING))
		return;

	gs_effect_set_int(filter->param_char_spacing, filter->char_spacing);
	gs_effect_set_bool(filter->param_char_size, filter->char_size);
	gs_effect_set_int(filter->param_color_mode, filter->color_mode);
	gs_effect_set_bool(filter->param_invert_brightness, filter->invert_brightness);
	gs_effect_set_bool(filter->param_dithering, filter->dithering);
	gs_effect_set_float(filter->param_dither_intensity, filter->dither_intensity);
	gs_effect_set_vec3(filter->param_bg_color, &filter->bg_color);
	gs_effect_set_bool(filter->param_calc_bg_color, filter->calc_bg_color);
	gs_effect_set_float(filter->param_char_color_scale, filter->char_color_scale);
	gs_effect_set_float(filter->param_bg_color_scale, filter->bg_color_scale);

	if (filter->font_size < 1) filter->font_size = 1;
	filter->width = (float)obs_source_get_base_width(
		obs_filter_get_target(filter->context))/filter->font_size;
	filter->height = (float)obs_source_get_base_height(
		obs_filter_get_target(filter->context))/filter->font_size;

	gs_effect_set_float(filter->param_width, filter->width);
	gs_effect_set_float(filter->param_height, filter->height);

	obs_source_process_filter_end(filter->context, filter->effect,	0, 0);

	UNUSED_PARAMETER(effect);
}

static uint32_t ascii_filter_width(void *data)
{
	struct ascii_filter_data *ascii = data;
	return (uint32_t)ascii->width;
}

static uint32_t ascii_filter_height(void *data)
{
	struct ascii_filter_data *ascii = data;
	return (uint32_t)ascii->height;
}

struct obs_source_info ascii_filter = {
	.id                            = "standalone_ascii_filter",
	.type                          = OBS_SOURCE_TYPE_FILTER,
	.output_flags                  = OBS_SOURCE_VIDEO,
	.get_name                      = ascii_filter_get_name,
	.create                        = ascii_filter_create,
	.destroy                       = ascii_filter_destroy,
	.update                        = ascii_filter_update,
	.get_properties                = ascii_filter_properties,
	.get_defaults                  = ascii_filter_defaults,
	.video_tick                    = ascii_filter_tick,
	.video_render                  = ascii_filter_render,
	.get_width                     = ascii_filter_width,
	.get_height                    = ascii_filter_height
};
