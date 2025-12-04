#pragma once
#include"renderer.h"
#include"color.h"
#include<vector>
#include<cmath>

namespace GraphicFunc
{
	namespace Rectang
	{
		void DrawRectangle(int x0, int y0, int x1, int y1, Renderer& renderer, Color color = Color(0.44f, 0.12f, 0.71f, 0.80f));
	}
}