#include"rectang.h"
#include"line.h"

namespace GraphicFunc
{
	namespace Rectang
	{
		void DrawRectangle(int x0, int y0, int x1, int y1, Renderer& renderer, Color color)
		{
			GraphicFunc::Line::DrawLineBresenham(x0, y0, x1, y0, renderer, color);
			GraphicFunc::Line::DrawLineBresenham(x0, y0, x0, y1, renderer, color);
			GraphicFunc::Line::DrawLineBresenham(x0, y1, x1, y1, renderer, color);
			GraphicFunc::Line::DrawLineBresenham(x1, y0, x1, y1, renderer, color);
		}
	}
}