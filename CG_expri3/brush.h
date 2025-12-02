#pragma once
#include"renderer.h"
#include"color.h"
#include<vector>
#include<cmath>

namespace GraphicFunc
{
	namespace Brush
	{
		void DrawFreeLine(int x0, int y0, int x1, int y1, Renderer& renderer);
	}
}