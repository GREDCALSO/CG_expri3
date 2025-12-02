#pragma once
#include"renderer.h"
#include"color.h"
#include<cmath>

namespace GraphicFunc
{
	namespace Line
	{
		void DrawLineBresenham(int x0, int y0, int x1, int y1, Renderer& renderer, Color color = { 1.0f,0.0f,0.0f,1.0f });
		void DrawLineMidpoint(int x0, int y0, int x1, int y1, Renderer& renderer);
	}
}
