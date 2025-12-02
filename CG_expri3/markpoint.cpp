#include"markpoint.h"

namespace GraphicFunc
{
	namespace MarkPoint
	{
		void DrawMarkPoint(int x, int y, Renderer& renderer)
		{
			Renderer::DrawScope scope(renderer);

			renderer.SetMark(x, y);
		}

		void CleanMarkPoint(const std::vector<POINT>& points, Renderer& renderer)
		{
			Renderer::DrawScope scope(renderer);
			//D2D1::ColorF COLOR = D2D1::ColorF::White;

			for (auto point : points)
			{
				renderer.SetMark(point.x, point.y, D2D1::ColorF::White);
			}
		}
	}
}