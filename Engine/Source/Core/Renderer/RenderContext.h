//
// Created by flode on 02/03/2023.
//

#ifndef ENGINE_RENDERCONTEXT_H
#define ENGINE_RENDERCONTEXT_H

#include "camera.h"

class light;
class ShaderProgram;

enum render_pass
{
	render_pass_main,
	render_pass_lighting,
	render_pass_custom,
};

struct RenderFlags
{
	bool visualize_hitboxes = false; //TODO: not really a render flag
};

//holds the information needed to render the current frame 
struct RenderContext
{
	camera *camera = nullptr;
	light *light = nullptr;
	double deltaTime = 0;
	RenderFlags flags;
	render_pass pass = render_pass_main;
	ShaderProgram *custom_shader = nullptr;
};

struct RenderWindow
{
	int width, height;
};

#endif //ENGINE_RENDERCONTEXT_H
