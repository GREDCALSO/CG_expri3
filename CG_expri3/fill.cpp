#include"fill.h"

namespace GraphicFunc
{
	namespace Fill
	{
		void FillPolygonScanline(const std::vector<POINT>& vertices, Renderer& renderer, Color fillColor)
		{
			Renderer::DrawScope scope(renderer);

			const int n = static_cast<int>(vertices.size());
			if (n < 3) return;

			// 全局 y 范围
			int globalMinY = vertices[0].y;
			int globalMaxY = vertices[0].y;
			//找到最大和最小Y坐标
			for (const auto& p : vertices)
			{
				if (p.y < globalMinY) globalMinY = p.y;
				if (p.y > globalMaxY) globalMaxY = p.y;
			}
			if (globalMinY == globalMaxY) return;	//如果再同一水平线上，无法填充

			// 构建边表（Bucket：以 yMin 为索引）
			//按边的起始Y坐标（yMin）分组存储所有非水平边
			const int bucketCount = globalMaxY - globalMinY + 1;
			std::vector<std::vector<ScanlineEdge>> ET(bucketCount);

			//添加边
			auto addEdge = [&](POINT a, POINT b)
				{
					// 忽略水平边（避免双计数）
					if (a.y == b.y) return;

					// 令 a 为较低端点（y小）
					if (a.y > b.y) std::swap(a, b);

					const int yMin = a.y;	//边的起始的Y坐标
					const int yMax = b.y - 1; // 边的结束的Y坐标；上端不计：避免顶点重复计入
					if (yMin > yMax) return;

					ScanlineEdge e;
					e.yMax = yMax;	//该边参与的最高扫描线
					e.x = static_cast<float>(a.x);	//当前扫描线与交点的X坐标（初始值）
					e.invSlope = static_cast<float>(b.x - a.x) / static_cast<float>(b.y - a.y);	//获取斜率
					ET[yMin - globalMinY].push_back(e);
				};

			//遍历控制点
			for (int i = 0; i < n; ++i)
			{
				POINT a = vertices[i];	//当前边起点
				POINT b = vertices[(i + 1) % n];	//当前边终点（首尾相接）
				addEdge(a, b);
			}

			std::vector<ScanlineEdge> AET;
			AET.reserve(16);

			// 扫描
			for (int y = globalMinY; y <= globalMaxY; ++y)
			{
				// 1) 将当前扫描线的边加入 AET
				const int bucketIndex = y - globalMinY;
				if (bucketIndex >= 0 && bucketIndex < bucketCount)
				{
					for (const auto& e : ET[bucketIndex]) AET.push_back(e);
				}

				// 2) 移除已失效的边（超过 yMax）
				AET.erase(std::remove_if(AET.begin(), AET.end(),
										 [y](const ScanlineEdge& e) { return y > e.yMax; }), AET.end());

				if (AET.empty()) continue;

				// 3) 按交点x排序
				std::sort(AET.begin(), AET.end(),
						  [](const ScanlineEdge& a, const ScanlineEdge& b)
						  {
							  if (a.x != b.x) return a.x < b.x;
							  return a.invSlope < b.invSlope;
						  });

				// 4) 成对填充
				for (size_t i = 0; i + 1 < AET.size(); i += 2)
				{
					//计算当前扫描线与两条边的交点X范围
					int xStart = static_cast<int>(std::ceil(AET[i].x));		//起始x，向上取整
					int xEnd = static_cast<int>(std::floor(AET[i + 1].x));	//结束x，向下取整
					if (xStart <= xEnd)
					{
						//在起始X和结束X之间填充像素
						for (int x = xStart; x <= xEnd; ++x)
							//renderer.SetPixel(x, y, fillColor);
							renderer.PlotPixel(x, y, fillColor);
					}
				}

				// 5) 更新AET中所有边的交点X坐标，x前进到下一条扫描线
				for (auto& e : AET) e.x += e.invSlope;
			}
		}

		void FillRandomShapeScanline(std::vector<POINT>& poly, Renderer& renderer)
		{
			if (poly.size() < 3)
			{
				poly.clear();
				return;
			}
			Color color(0.73f, 0.46f, 0.81f, 1.0f);
			FillPolygonScanline(poly, renderer, color);
			poly.clear();
		}

		void FillPolygonFence(const std::vector<POINT>& vertices, Renderer& renderer, Color fillColor)
		{
			Renderer::DrawScope scope(renderer);

			const int n = static_cast<int>(vertices.size());
			if (n < 3) return;  // 至少三点才能形成封闭区域

			// 计算 Y 范围
			int yMin = vertices[0].y;
			int yMax = vertices[0].y;
			for (const auto& p : vertices)
			{
				if (p.y < yMin) yMin = p.y;
				if (p.y > yMax) yMax = p.y;
			}

			// 对每条扫描线
			for (int y = yMin; y <= yMax; ++y)
			{
				//存储当前扫描线与多边形各边的交点X坐标
				std::vector<float> intersections;

				// 计算与多边形边的交点
				for (int i = 0; i < n; ++i)
				{
					POINT p1 = vertices[i];
					POINT p2 = vertices[(i + 1) % n];

					// 忽略水平边，因为水平边不会产生单一的交点
					if (p1.y == p2.y) continue;

					// 确保 p1.y < p2.y
					if (p1.y > p2.y) std::swap(p1, p2);

					// 若扫描线在边的范围内，计算交点
					if (y >= p1.y && y < p2.y)
					{
						//使用线性插值计算交点X坐标
						float x = p1.x + (float)(y - p1.y) * (float)(p2.x - p1.x) / (float)(p2.y - p1.y);
						intersections.push_back(x);
					}
				}

				// 没有交点 → 跳过
				if (intersections.empty()) continue;

				// 按 x 排序
				std::sort(intersections.begin(), intersections.end());

				// 奇偶规则：两两成对填充
				//从多边形外部开始，遇到第一个交点进入内部，遇到第二个交点回到外部，依此类推
				for (size_t i = 0; i + 1 < intersections.size(); i += 2)
				{
					int xStart = static_cast<int>(std::ceil(intersections[i]));
					int xEnd = static_cast<int>(std::floor(intersections[i + 1]));
					for (int x = xStart; x <= xEnd; ++x)
						renderer.PlotPixel(x, y, fillColor);
				}
			}
		}

		void FillRandomShapeFence(std::vector<POINT>& poly, Renderer& renderer)
		{
			if (poly.size() < 3)
			{
				poly.clear();
				return;
			}
			Color color(0.96f, 0.76f, 0.26f, 1.0f); // 黄色
			FillPolygonFence(poly, renderer, color);
			poly.clear();
		}
	}
}