#pragma once
#include"renderer.h"
#include"color.h"
#include<vector>
#include<cmath>

namespace GraphicFunc
{
	namespace Polygon
	{
		void DrawRandomPolygon(std::vector<POINT>& vertices, Renderer& renderer, Color color = Color(0.3f, 0.21f, 0.64f, 1.0f));
	}
}