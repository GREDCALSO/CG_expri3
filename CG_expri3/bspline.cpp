#include"bspline.h"
#include"line.h"

namespace GraphicFunc
{
	namespace Bspline
	{
		void DrawBSpline(const std::vector<POINT>& ctrlPts, Renderer& renderer, Color color)
		{
			Renderer::DrawScope scope(renderer);
			const int n = static_cast<int>(ctrlPts.size());
			if (n < 4) return;  // 至少4个控制点

			//使用3次B样条曲线基函数计算型值点
			auto splinePoint = [&](float u, const POINT& p0, const POINT& p1, const POINT& p2, const POINT& p3) -> POINT
				{
					float u2 = u * u;
					float u3 = u2 * u;
					//P(u) = b0(u)P0 + b1(u)P1 + b2(u)P2 + b3(u)P3
					float b0 = (-u3 + 3 * u2 - 3 * u + 1) / 6.0f;
					float b1 = (3 * u3 - 6 * u2 + 4) / 6.0f;
					float b2 = (-3 * u3 + 3 * u2 + 3 * u + 1) / 6.0f;
					float b3 = u3 / 6.0f;
					POINT pt;
					//计算曲线上点的坐标：各控制点坐标乘以对应的基函数权重后求和
					pt.x = static_cast<int>(b0 * p0.x + b1 * p1.x + b2 * p2.x + b3 * p3.x);
					pt.y = static_cast<int>(b0 * p0.y + b1 * p1.y + b2 * p2.y + b3 * p3.y);
					return pt;
				};

			// 每4个点形成一段曲线
			for (int i = 1; i < n - 2; ++i)
			{
				//计算当前曲线段在参数u=0时的起点
				POINT prev = splinePoint(0.0f, ctrlPts[i - 1], ctrlPts[i], ctrlPts[i + 1], ctrlPts[i + 2]);
				//从u=0.01到u=1.0，以0.01为步长计算曲线上的点
				for (float u = 0.01f; u <= 1.0f; u += 0.01f)
				{
					//计算当前参数u对应的曲线点
					POINT curr = splinePoint(u, ctrlPts[i - 1], ctrlPts[i], ctrlPts[i + 1], ctrlPts[i + 2]);
					//用直线段连接相邻的曲线点，形成光滑曲线
					GraphicFunc::Line::DrawLineBresenham(prev.x, prev.y, curr.x, curr.y, renderer, color);
					//更新前一个点位置
					prev = curr;
				}
			}
		}

		void DrawRandomBSpline(std::vector<POINT>& ctrlPts, Renderer& renderer)
		{
			if (ctrlPts.size() < 4) return;
			Color color(0.3f, 0.0f, 0.0f, 0.2f);
			for (int i = 0; i < ctrlPts.size() - 1; i++)
			{
				GraphicFunc::Line::DrawLineBresenham(ctrlPts[i].x, ctrlPts[i].y, ctrlPts[i + 1].x, ctrlPts[i + 1].y, renderer, color);
			}
			DrawBSpline(ctrlPts, renderer);
			ctrlPts.clear();
		}
	}
}