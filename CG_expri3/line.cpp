#include"line.h"

namespace GraphicFunc
{
	namespace Line
	{
		void DrawLineBresenham(int x0, int y0, int x1, int y1, Renderer& renderer, Color color)
		{
			Renderer::DrawScope scope(renderer);
			//Bresenham，使用整数运算和误差项来决定下一个像素的位置
			//坐标差值
			int dx = abs(x1 - x0);
			int dy = abs(y1 - y0);

			//步进
			int sx = (x0 < x1) ? 1 : -1;
			int sy = (y0 < y1) ? 1 : -1;
			int err = dx - dy;	//误差项

			while (true)
			{
				renderer.PlotPixel(x0, y0, color);
				if (x0 == x1 && y0 == y1) break;
				int e2 = 2 * err;	//两倍误差项（避免除法）
				//如果 2×err > -Δy：x方向步进，误差减少Δy
				//如果 2×err < Δx：y方向步进，误差增加Δx
				if (e2 > -dy)
				{
					err -= dy;
					x0 += sx;	//x步进
				}
				if (e2 < dx)
				{
					err += dx;
					y0 += sy;
				}
			}
		}

		void DrawLineMidpoint(int x0, int y0, int x1, int y1, Renderer& renderer)
		{
			Renderer::DrawScope scope(renderer);

			int dx = abs(x1 - x0);
			int dy = abs(y1 - y0);
			int sx = (x0 < x1) ? 1 : -1;
			int sy = (y0 < y1) ? 1 : -1;

			int x = x0, y = y0;

			if (dx > dy)
			{
				int d = 2 * dy - dx;
				for (int i = 0; i < dx; i++)
				{
					renderer.PlotPixel(x, y, Color(0.0f, 0.0f, 1.0f, 1.0f));
					x += sx;
					if (d < 0)
					{
						d += 2 * dy;
					}
					else
					{
						y += sy;
						d += 2 * (dy - dx);
					}
				}
			}
			else
			{
				int d = 2 * dx - dy;
				for (int i = 0; i <= dy; i++)
				{
					renderer.PlotPixel(x, y, Color(0.0f, 0.0f, 1.0f, 1.0f));
					y += sy;
					if (d < 0)
					{
						d += 2 * dx;
					}
					else
					{
						x += sx;
						d += 2 * (dx - dy);
					}
				}
			}
		}
	}
}