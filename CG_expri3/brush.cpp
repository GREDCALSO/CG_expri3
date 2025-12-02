#include"brush.h"
#include"line.h"

namespace GraphicFunc
{
	namespace Brush
	{
		void DrawFreeLine(int x0, int y0, int x1, int y1, Renderer& renderer)
		{
			//Color color(0.66f, 0.88f, 0.28f, 0.75f);
			GraphicFunc::Line::DrawLineBresenham(x0, y0, x1, y1, renderer);
		}
	}
}