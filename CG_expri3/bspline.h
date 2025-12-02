#pragma once
#include"renderer.h"
#include"color.h"
#include<vector>
#include<algorithm>
#include<cmath>

namespace GraphicFunc
{
	namespace Bspline
	{
		void DrawBSpline(const std::vector<POINT>& ctrlPts, Renderer& renderer, Color color = Color(0.1f, 0.2f, 0.3f, 1.0f));
		void DrawRandomBSpline(std::vector<POINT>& ctrlPts, Renderer& renderer);
	}
}