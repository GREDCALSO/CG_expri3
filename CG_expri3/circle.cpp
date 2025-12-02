#include"circle.h"

namespace GraphicFunc
{
	namespace Circle
	{
		void plotCirclePoints(int xc, int yc, int x, int y, std::vector<std::pair<int, int>>& points)
		{
			points.push_back({ xc + x, yc + y });
			points.push_back({ xc - x, yc + y });
			points.push_back({ xc + x, yc - y });
			points.push_back({ xc - x, yc - y });
			points.push_back({ xc + y, yc + x });
			points.push_back({ xc - y, yc + x });
			points.push_back({ xc + y, yc - x });
			points.push_back({ xc - y, yc - x });
		}

		void DrawCircleMidpoint(int x0, int y0, int x1, int y1, Renderer& renderer)
		{
			Renderer::DrawScope scope(renderer);

			int xc = (x1 + x0) / 2;
			int yc = (y1 + y0) / 2;
			double dx = static_cast<double>(x1 - x0);
			double dy = static_cast<double>(y1 - y0);
			int R = static_cast<int>(std::round(std::sqrt(dx * dx + dy * dy) / 2.0));
			if (R <= 0) return;

			std::vector<std::pair<int, int>> points;

			int x = 0;
			int y = R;
			int d = 1 - R;

			plotCirclePoints(xc, yc, x, y, points);
			Color color(0.225f, 0.66f, 0.39f, 1.0f);

			while (x < y)
			{
				x++;
				if (d < 0)
				{
					d += 2 * x + 3;
				}
				else
				{
					y--;
					d += 2 * (x - y) + 5;
				}
				plotCirclePoints(xc, yc, x, y, points);
			}

			for (const auto& point : points)
			{
				renderer.PlotPixel(point.first, point.second, color);
			}
		}

		void DrawCircleBresenham(int x0, int y0, int x1, int y1, Renderer& renderer)
		{
			Renderer::DrawScope scope(renderer);

			int xc = (x1 + x0) / 2;
			int yc = (y1 + y0) / 2;
			double dx = static_cast<double>(x1 - x0);
			double dy = static_cast<double>(y1 - y0);
			int R = static_cast<int>(std::round(std::sqrt(dx * dx + dy * dy) / 2.0));
			if (R <= 0) return;

			int x = 0;
			int y = R;
			int d = 3 - 2 * R;
			Color color(0.576f, 0.478f, 0.11f, 0.8f);

			std::vector<std::pair<int, int>> points;

			plotCirclePoints(xc, yc, x, y, points);

			while (x <= y)
			{
				x++;
				if (d < 0)
				{
					d = d + 4 * x + 6;
				}
				else
				{
					d = d + 4 * (x - y) + 10;
					y--;
				}
				plotCirclePoints(xc, yc, x, y, points);
			}

			for (const auto& point : points)
			{
				renderer.PlotPixel(point.first, point.second, color);
			}
		}
	}
}