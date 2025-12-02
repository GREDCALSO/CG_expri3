#include"polygon.h"
#include"line.h"

namespace GraphicFunc
{
	namespace Polygon
	{
		void DrawRandomPolygon(std::vector<POINT>& vertices, Renderer& renderer, Color color)
		{
			if (vertices.size() < 2) return;
			int i = 0;
			for (i = 0; i < vertices.size() - 1; i++)
			{
				GraphicFunc::Line::DrawLineBresenham(vertices[i].x, vertices[i].y, vertices[i + 1].x, vertices[i + 1].y, renderer, color);
			}
			GraphicFunc::Line::DrawLineBresenham(vertices[i].x, vertices[i].y, vertices[0].x, vertices[0].y, renderer, color);
			vertices.clear();
		}
	}
}