#include"rectang.h"
#include"line.h"

namespace GraphicFunc
{
	namespace Rectang
	{
		void DrawRectangle(int x0, int y0, int x1, int y1, Renderer& renderer)
		{
			Color color(0.44f, 0.12f, 0.71f, 0.80f);
			GraphicFunc::Line::DrawLineBresenham(x0, y0, x1, y0, renderer, color);
			GraphicFunc::Line::DrawLineBresenham(x0, y0, x0, y1, renderer, color);
			GraphicFunc::Line::DrawLineBresenham(x0, y1, x1, y1, renderer, color);
			GraphicFunc::Line::DrawLineBresenham(x1, y0, x1, y1, renderer, color);
		}
	}
}