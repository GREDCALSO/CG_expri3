#pragma once
#include"renderer.h"
#include"color.h"
#include<vector>
#include<cmath>

namespace GraphicFunc
{
	namespace Circle
	{
		void plotCirclePoints(int xc, int yc, int x, int y, std::vector<std::pair<int, int>>& points);
		void DrawCircleMidpoint(int x0, int y0, int x1, int y1, Renderer& renderer);
		void DrawCircleBresenham(int x0, int y0, int x1, int y1, Renderer& renderer);
	}
}