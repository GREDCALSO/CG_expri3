#pragma once
#include"renderer.h"
#include<vector>

namespace GraphicFunc
{
	namespace MarkPoint
	{
		void DrawMarkPoint(int x, int y, Renderer& renderer);
		void CleanMarkPoint(const std::vector<POINT>& points, Renderer& renderer);
	}
}