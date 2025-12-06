#include "clip_polygon.h"
#include <cmath>
#include <algorithm>
#include <limits>
#include <cassert>
#include <list>
#include <unordered_set>

namespace GraphicFunc
{
	namespace Clip
	{

		// ---------- 工具函数 ----------
		static inline bool PointInsideRect(const POINT& p, const POINT& left_top, const POINT& right_bottom) {
			return p.x >= left_top.x && p.x <= right_bottom.x && p.y >= left_top.y && p.y <= right_bottom.y;
		}

		// 线段 AB 与 垂直/水平线（x=const 或 y=const）求交点（假定有交且不平行）
		static POINT IntersectSegmentWithVertical(const POINT& a, const POINT& b, int x) {
			double dx = (double)(b.x - a.x);
			double dy = (double)(b.y - a.y);
			if (fabs(dx) < 1e-9)
			{ // 垂直段（退化）直接返回端点之一
				return a;
			}
			double t = (x - a.x) / dx;
			double y = a.y + t * dy;
			return POINT { x, (int)std::round(y) };
		}

		static POINT IntersectSegmentWithHorizontal(const POINT& a, const POINT& b, int y) {
			double dx = (double)(b.x - a.x);
			double dy = (double)(b.y - a.y);
			if (fabs(dy) < 1e-9)
			{
				return a;
			}
			double t = (y - a.y) / dy;
			double x = a.x + t * dx;
			return POINT { (int)std::round(x), y };
		}

		// 插入点时避免重复非常接近的点
		static bool IsSamePoint(const POINT& a, const POINT& b) {
			return a.x == b.x && a.y == b.y;
		}

		// ---------- Sutherland-Hodgman 实现 ----------
		// 边界枚举，按照顺序：左、右、上、下（任意顺序均可，但实现里保持一致）
		enum class ClipEdge { Left, Right, Top, Bottom };

		static bool Inside(const POINT& p, ClipEdge edge, const POINT& left_top, const POINT& right_bottom) {
			switch (edge)
			{
			case ClipEdge::Left:   return p.x >= left_top.x;
			case ClipEdge::Right:  return p.x <= right_bottom.x;
			case ClipEdge::Top:    return p.y >= left_top.y;
			case ClipEdge::Bottom: return p.y <= right_bottom.y;
			default: return false;
			}
		}

		static POINT ComputeIntersection(const POINT& a, const POINT& b, ClipEdge edge, const POINT& left_top, const POINT& right_bottom) {
			switch (edge)
			{
			case ClipEdge::Left:
				return IntersectSegmentWithVertical(a, b, left_top.x);
			case ClipEdge::Right:
				return IntersectSegmentWithVertical(a, b, right_bottom.x);
			case ClipEdge::Top:
				return IntersectSegmentWithHorizontal(a, b, left_top.y);
			case ClipEdge::Bottom:
				return IntersectSegmentWithHorizontal(a, b, right_bottom.y);
			default:
				return a;
			}
		}

		std::vector<POINT> SutherlandHodgmanClip(const std::vector<POINT>& subject,
												 const POINT& left_top,
												 const POINT& right_bottom)
		{
			std::vector<POINT> input = subject;
			if (input.empty()) return {};

			ClipEdge edges[4] = { ClipEdge::Left, ClipEdge::Right, ClipEdge::Top, ClipEdge::Bottom };

			for (int ei = 0; ei < 4; ++ei)
			{
				ClipEdge edge = edges[ei];
				std::vector<POINT> output;
				size_t n = input.size();
				if (n == 0)
				{
					input.swap(output);
					break;
				}

				for (size_t i = 0; i < n; ++i)
				{
					const POINT& cur = input[i];
					const POINT& prev = input[(i + n - 1) % n];

					bool curInside = Inside(cur, edge, left_top, right_bottom);
					bool prevInside = Inside(prev, edge, left_top, right_bottom);

					if (prevInside && curInside)
					{
						// 都在：加入当前点
						if (output.empty() || !IsSamePoint(output.back(), cur))
							output.push_back(cur);
					}
					else if (prevInside && !curInside)
					{
						// 出边：加入交点
						POINT ip = ComputeIntersection(prev, cur, edge, left_top, right_bottom);
						if (output.empty() || !IsSamePoint(output.back(), ip))
							output.push_back(ip);
					}
					else if (!prevInside && curInside)
					{
						// 进边：先加入交点，再加入当前点
						POINT ip = ComputeIntersection(prev, cur, edge, left_top, right_bottom);
						if (output.empty() || !IsSamePoint(output.back(), ip))
							output.push_back(ip);
						if (output.empty() || !IsSamePoint(output.back(), cur))
							output.push_back(cur);
					}
					else
					{
						// 都在外：什么都不做
					}
				}

				input.swap(output);
			}

			// 若最后结果首尾重复，则整理
			if (input.size() > 1 && IsSamePoint(input.front(), input.back()))
			{
				input.pop_back();
			}

			// 可能产生很短的退化多边形，过滤
			if (input.size() < 3)
			{
				// 退化为线或点：我们认为为空（UI上会显示为空）
				return {};
			}
			return input;
		}

		// ========== Weiler-Atherton 实现 ==========

		// 简单的点结构（用于WA算法）
		struct WAPoint {
			double x, y;
			bool isIntersection;
			bool isEntering;
			int edgeIdx;  // 所属边的索引
			double t;     // 在边上的参数位置
			
			WAPoint(double px = 0, double py = 0) 
				: x(px), y(py), isIntersection(false), isEntering(false), edgeIdx(-1), t(0.0) {}
		};

		// 判断点是否在矩形内
		static inline bool PointInRectWA(double x, double y, const POINT& lt, const POINT& rb) {
			return x >= lt.x && x <= rb.x && y >= lt.y && y <= rb.y;
		}

		// 线段求交
		static bool GetSegmentIntersection(double x1, double y1, double x2, double y2,
										   double x3, double y3, double x4, double y4,
										   double& t, double& ix, double& iy) {
			double dx1 = x2 - x1, dy1 = y2 - y1;
			double dx2 = x4 - x3, dy2 = y4 - y3;
			double cross = dx1 * dy2 - dy1 * dx2;
			
			const double EPS = 1e-9;
			if (fabs(cross) < EPS) return false;
			
			double dx3 = x3 - x1, dy3 = y3 - y1;
			t = (dx3 * dy2 - dy3 * dx2) / cross;
			double t2 = (dx3 * dy1 - dy3 * dx1) / cross;
			
			if (t > EPS && t < 1.0 - EPS && t2 > EPS && t2 < 1.0 - EPS) {
				ix = x1 + t * dx1;
				iy = y1 + t * dy1;
				return true;
			}
			return false;
		}

		std::vector<std::vector<POINT>> WeilerAthertonClip(const std::vector<POINT>& subject,
														   const POINT& left_top,
														   const POINT& right_bottom)
		{
			std::vector<std::vector<POINT>> result;
			if (subject.size() < 3) return result;

			// 构建带交点的顶点列表
			std::vector<WAPoint> vertices;
			size_t n = subject.size();
			
			// 定义clip窗口的四条边（顺时针）
			POINT clipCorners[4] = {
				{left_top.x, left_top.y},           // 左上
				{right_bottom.x, left_top.y},       // 右上
				{right_bottom.x, right_bottom.y},   // 右下
				{left_top.x, right_bottom.y}        // 左下
			};

			// 遍历subject的每条边
			for (size_t i = 0; i < n; ++i) {
				size_t next = (i + 1) % n;
				POINT p1 = subject[i];
				POINT p2 = subject[next];
				
				// 添加起点
				WAPoint v1(p1.x, p1.y);
				v1.edgeIdx = (int)i;
				vertices.push_back(v1);
				
				// 找出该边与clip窗口的所有交点
				std::vector<WAPoint> edgeIntersections;
				
				for (int j = 0; j < 4; ++j) {
					POINT c1 = clipCorners[j];
					POINT c2 = clipCorners[(j + 1) % 4];
					
					double t, ix, iy;
					if (GetSegmentIntersection(p1.x, p1.y, p2.x, p2.y,
											  c1.x, c1.y, c2.x, c2.y,
											  t, ix, iy)) {
						WAPoint inter(ix, iy);
						inter.isIntersection = true;
						inter.t = t;
						inter.edgeIdx = (int)i;
						
						// 判断是进入还是离开：检查p1是否在内部
						bool p1Inside = PointInRectWA(p1.x, p1.y, left_top, right_bottom);
						inter.isEntering = !p1Inside;
						
						edgeIntersections.push_back(inter);
					}
				}
				
				// 按t排序并添加交点
				std::sort(edgeIntersections.begin(), edgeIntersections.end(),
						 [](const WAPoint& a, const WAPoint& b) { return a.t < b.t; });
				
				for (const auto& inter : edgeIntersections) {
					vertices.push_back(inter);
				}
			}

			// 检查是否有交点
			bool hasIntersection = false;
			for (const auto& v : vertices) {
				if (v.isIntersection) {
					hasIntersection = true;
					break;
				}
			}

			// 没有交点：检查是否完全在内部
			if (!hasIntersection) {
				bool allInside = true;
				for (const auto& p : subject) {
					if (!PointInRectWA(p.x, p.y, left_top, right_bottom)) {
						allInside = false;
						break;
					}
				}
				if (allInside) {
					result.push_back(subject);
				}
				return result;
			}

			// Weiler-Atherton算法：从每个未访问的进入交点开始独立遍历
			std::vector<bool> visited(vertices.size(), false);
			
			for (size_t start = 0; start < vertices.size(); ++start) {
				// 只从未访问的进入交点开始
				if (visited[start] || !vertices[start].isIntersection || !vertices[start].isEntering) {
					continue;
				}

				std::vector<POINT> polygon;
				size_t curr = start;
				int maxIter = (int)vertices.size() * 2;
				int iter = 0;

				do {
					visited[curr] = true;
					
					// 添加当前点
					POINT pt;
					pt.x = (LONG)std::round(vertices[curr].x);
					pt.y = (LONG)std::round(vertices[curr].y);
					
					if (polygon.empty() || polygon.back().x != pt.x || polygon.back().y != pt.y) {
						polygon.push_back(pt);
					}

					// 沿subject边前进到下一个顶点
					size_t next = (curr + 1) % vertices.size();
					
					// 检查下一个点是否是离开交点
					if (vertices[next].isIntersection && !vertices[next].isEntering) {
						// 遇到离开交点，添加它并结束当前多边形
						visited[next] = true;
						
						POINT leavePt;
						leavePt.x = (LONG)std::round(vertices[next].x);
						leavePt.y = (LONG)std::round(vertices[next].y);
						
						if (polygon.empty() || polygon.back().x != leavePt.x || polygon.back().y != leavePt.y) {
							polygon.push_back(leavePt);
						}
						
						// 关键修改：结束当前多边形，不再继续寻找下一个进入点
						break;
					}
					
					curr = next;
					iter++;
					
				} while (curr != start && iter < maxIter);

				// 去除首尾重复
				if (polygon.size() > 1 && polygon[0].x == polygon.back().x && 
					polygon[0].y == polygon.back().y) {
					polygon.pop_back();
				}

				// 只添加有效的多边形（至少3个顶点）
				if (polygon.size() >= 3) {
					result.push_back(polygon);
				}
			}

			return result;
		}


	} // namespace Clip
} // namespace GraphicFunc
