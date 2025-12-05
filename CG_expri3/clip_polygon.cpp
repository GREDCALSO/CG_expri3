#include "clip_polygon.h"
#include <cmath>
#include <algorithm>
#include <limits>
#include <cassert>

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

		// ---------- Weiler–Atherton 实现（针对凸裁剪多边形 - 矩形） ----------
		struct Intersection
		{
			POINT pt;
			int subjEdge;    // subject 边索引 i 表示 s[i] -> s[i+1]
			double subjT;    // 在该边上的参数 0..1
			int clipEdge;    // 裁剪多边形的边索引 j 表示 c[j] -> c[j+1]
			double clipT;    // 在 clip 边上的参数 0..1
			bool entering;   // 这是 entering 还是 leaving（相对 subject 边方向）
			bool used = false;
		};

		// 计算线段 AB 和 CD 是否相交（严格区间(0,1) 或包括端点根据需要）
		// 返回是否相交，以及交点参数 t along AB, u along CD
		static bool SegmentIntersectParam(const POINT& A, const POINT& B, const POINT& C, const POINT& D, double& t_out, double& u_out) {
			// 2D segment intersection using parametric form
			double x1 = A.x, y1 = A.y;
			double x2 = B.x, y2 = B.y;
			double x3 = C.x, y3 = C.y;
			double x4 = D.x, y4 = D.y;

			double denom = (y4 - y3) * (x2 - x1) - (x4 - x3) * (y2 - y1);
			if (fabs(denom) < 1e-9) return false; // 平行或重合（我们忽略重合的复杂情形）

			double t = ((x4 - x3) * (y1 - y3) - (y4 - y3) * (x1 - x3)) / denom;
			double u = ((x2 - x1) * (y1 - y3) - (y2 - y1) * (x1 - x3)) / denom;

			// 允许端点交（t,u ∈ [0,1]）
			if (t < -1e-9 || t > 1.0 + 1e-9 || u < -1e-9 || u > 1.0 + 1e-9) return false;
			t_out = min(max(t, 0.0), 1.0);
			u_out = min(max(u, 0.0), 1.0);
			return true;
		}

		std::vector<std::vector<POINT>> WeilerAthertonClip(const std::vector<POINT>& subject,
														   const POINT& left_top,
														   const POINT& right_bottom)
		{
			std::vector<std::vector<POINT>> results;

			if (subject.size() < 3) return results;

			// 定义裁剪矩形（按顺时针或逆时针都可以，但保持顺序）
			std::vector<POINT> clipPoly;
			// 顺时针：left_top -> right_top -> right_bottom -> left_bottom
			clipPoly.push_back(left_top);
			clipPoly.push_back(POINT { right_bottom.x, left_top.y }); // right_top
			clipPoly.push_back(right_bottom);
			clipPoly.push_back(POINT { left_top.x, right_bottom.y }); // left_bottom

			size_t sn = subject.size();
			size_t cn = clipPoly.size();

			// 1) 先判断全部在内/全部在外的简单情形
			bool anyInside = false;
			bool anyOutside = false;
			for (const POINT& p : subject)
			{
				if (PointInsideRect(p, left_top, right_bottom)) anyInside = true;
				else anyOutside = true;
			}
			if (!anyInside)
			{
				// 全部外 -> 空
				return results;
			}
			if (!anyOutside)
			{
				// 全部内 -> 原样返回
				results.push_back(subject);
				return results;
			}

			// 2) 计算所有交点
			std::vector<Intersection> inters;

			for (size_t i = 0; i < sn; ++i)
			{
				POINT A = subject[i];
				POINT B = subject[(i + 1) % sn];

				for (size_t j = 0; j < cn; ++j)
				{
					POINT C = clipPoly[j];
					POINT D = clipPoly[(j + 1) % cn];

					double t, u;
					if (SegmentIntersectParam(A, B, C, D, t, u))
					{
						// 过滤掉在端点极端数值误差情况下产生的重复交点
						POINT ip;
						ip.x = (int)std::round(A.x + t * (B.x - A.x));
						ip.y = (int)std::round(A.y + t * (B.y - A.y));
						Intersection I;
						I.pt = ip;
						I.subjEdge = (int)i;
						I.subjT = t;
						I.clipEdge = (int)j;
						I.clipT = u;
						// entering/leaving 判断：如果从 A->B 的方向，A 在外 B 在内 为 entering
						bool Ain = PointInsideRect(A, left_top, right_bottom);
						bool Bin = PointInsideRect(B, left_top, right_bottom);
						I.entering = (!Ain && Bin);
						inters.push_back(I);
					}
				}
			}

			if (inters.empty())
			{
				// 没有交点，且既有内点又有外点（理论上不会发生，但为保险）
				// 如果某个顶点内则返回连通的内点片段（简化策略：用 SH 覆盖）
				auto fallback = SutherlandHodgmanClip(subject, left_top, right_bottom);
				if (!fallback.empty()) results.push_back(fallback);
				return results;
			}

			// 将交点按 subject 边与 t 值进行分桶排序
			// map from subjEdge to vector of indices into inters
			std::vector<std::vector<int>> subjBuckets(sn);
			std::vector<std::vector<int>> clipBuckets(cn);

			for (int idx = 0; idx < (int)inters.size(); ++idx)
			{
				const Intersection& I = inters[idx];
				subjBuckets[I.subjEdge].push_back(idx);
				clipBuckets[I.clipEdge].push_back(idx);
			}

			// 按 t / u 排序
			for (size_t i = 0; i < subjBuckets.size(); ++i)
			{
				auto& v = subjBuckets[i];
				std::sort(v.begin(), v.end(), [&](int a, int b)
						  {
							  return inters[a].subjT < inters[b].subjT;
						  });
			}
			for (size_t j = 0; j < clipBuckets.size(); ++j)
			{
				auto& v = clipBuckets[j];
				std::sort(v.begin(), v.end(), [&](int a, int b)
						  {
							  return inters[a].clipT < inters[b].clipT;
						  });
			}

			// 需要为每个交点建立“配对”关系：同一交点在 subjBuckets 与 clipBuckets 中的索引表示同一个对象 inters[k]
			// WA 遍历时使用 inters[k].used 标记是否被切片使用。

			// 3) 使用交点追踪环路
			for (int startIdx = 0; startIdx < (int)inters.size(); ++startIdx)
			{
				if (inters[startIdx].used) continue;

				// 从一个未访问交点开始构造一个输出环
				std::vector<POINT> outPoly;
				int curIdx = startIdx;

				// 如果 starting intersection 是 leaving（从内到外），更常见的 WA 策略是从 entering 开始以得到顺序圈。
				// 但不强制：也能正常处理。我们统一从一个 entering 开始（如果当前不是 entering，找到同边的下一个交点作为起点）
				if (!inters[curIdx].entering)
				{
					// 尝试在同一 subjEdge 找一个 entering
					bool found = false;
					int se = inters[curIdx].subjEdge;
					for (int id : subjBuckets[se])
					{
						if (inters[id].entering) { curIdx = id; found = true; break; }
					}
					if (!found)
					{
						// 找不到则从当前继续
					}
				}

				// 保存起点，用于终止判断
				int firstIdx = curIdx;

				while (true)
				{
					// 1) 把当前交点加入输出，并标记为 used
					inters[curIdx].used = true;
					outPoly.push_back(inters[curIdx].pt);

					// 2) 沿 subject 从当前交点前进到下一个交点（或回到起点）
					// 当前在 subject 的某条边 s[e] -> s[e+1]，并处于该边上的某个 t
					int e = inters[curIdx].subjEdge;
					// 在 subjBuckets[e] 中找到 curIdx 的位置
					auto& sb = subjBuckets[e];
					int posInSb = -1;
					for (int k = 0; k < (int)sb.size(); ++k) if (sb[k] == curIdx) { posInSb = k; break; }
					// 下一交点沿 subject 方向（t 增大）
					int nextIdxOnSubject = -1;
					if (posInSb != -1 && posInSb + 1 < (int)sb.size())
					{
						nextIdxOnSubject = sb[posInSb + 1];
						// 将 subject 边上从交点到下交点之间的顶点加入（如果有顶点）
						// Add subject vertices between these intersections:
						POINT nextPt = inters[nextIdxOnSubject].pt;
						// 从当前 edge 的交点走到 next交点，可能会经过后续顶点：
						// 把当前交点所在的边的末端点（subject[e+1]）加入，并继续加入后续顶点直到到达 next交点 的所在边
						// 但简化操作：先在 subject 上从交点继续走，添加后续的 subject 顶点，直到刚到 nextIdxOnSubject 的交点前
						// 计算 steps:
						int curVertexIndexAfter = (e + 1) % (int)sn;
						// 加入边末端点
						// 但要避免重复插入与靠近交点的点
						// 插入 subject[curVertexIndexAfter]，然后检查后续边是否包含下一个交点的边
						int walkIdx = curVertexIndexAfter;
						while (true)
						{
							// 如果下一个交点在同一边并且 t 在该边上高于当前，则我们在该边上就应该在到达下交点前结束
							// 我们把走到下交点的前一个顶点都加上
							// 首先判断是否 nextIdxOnSubject 属于当前正在走的边
							int nextEdgeIdx = inters[nextIdxOnSubject].subjEdge;
							if (walkIdx == nextEdgeIdx)
							{
								// 到达包含下交点的边，停止在加入下交点之前
								break;
							}
							// 否则把该顶点加入输出并继续
							if (outPoly.empty() || !IsSamePoint(outPoly.back(), subject[walkIdx]))
								outPoly.push_back(subject[walkIdx]);
							walkIdx = (walkIdx + 1) % (int)sn;
							// 防止意外无限循环（保险）
							if (walkIdx == curVertexIndexAfter) break;
						}
						// 将下一个交点本身加入（下一步 loop 会处理并标记）
						curIdx = nextIdxOnSubject;
						continue;
					}
					else
					{
						// 当前交点为该边上的最后一个交点： 沿 subject 前进穿过顶点，直到碰到下一个交点（在后面的边）
						// 在这个情况下，需要沿着 subject 加入从当前交点所在边的末端开始的顶点，直到遇到某个边上有交点（按顺序）
						int walkEdge = e;
						int walkVert = (e + 1) % (int)sn;
						bool foundNext = false;
						for (int step = 0; step < (int)sn; ++step)
						{
							// 如果边 walkEdge 上有交点，取该边第一个交点（按 t 升序）
							if (!subjBuckets[walkEdge].empty())
							{
								int candidate = subjBuckets[walkEdge][0];
								nextIdxOnSubject = candidate;
								foundNext = true;
								break;
							}
							// 否则把该边的末端顶点加入并继续
							if (outPoly.empty() || !IsSamePoint(outPoly.back(), subject[walkVert]))
								outPoly.push_back(subject[walkVert]);
							walkEdge = (walkEdge + 1) % (int)sn;
							walkVert = (walkEdge + 1) % (int)sn;
						}
						if (!foundNext)
						{
							// 没有更多交点了：理论上不该发生（因为我们知道有交点），但作为兜底，把路径回连并终止
							break;
						}
						curIdx = nextIdxOnSubject;
						continue;
					}

					// 在到达下一个交点（curIdx 已更新为下一个交点）后，切换到沿 clip polygon 走
					// 3) 找到在 clip polygon 上对应的交点索引 curIdx（inters[curIdx].clipEdge, clipT）
					// 沿 clip polygon（按顺序）从该交点出发走到下一个交点（沿裁剪边顺时针/逆时针方向），并把沿途的 clip 顶点加入输出
					// 首先在 clipBuckets[inters[curIdx].clipEdge] 中找到 curIdx 位置
					// 但是上面逻辑流在某些路径里已经用 continue 处理下一 curIdx； 所以下面这段在程序逻辑上作为备用/互补
					// 这里我们改用：直接切换到 clip 路径，从当前在 subject 的交点对应的 clipEdge 找到其在 clipBuckets 的位置
					// 然后取该 clipEdge 的下一个交点（按 clipT 升序），沿往 clip 顶点走，直到到达 nextClipIntersection
					// 实际上因为上面逻辑通过 continue 已经把 curIdx 更新为下一个 subject 交点，此处留作保险
					break;
				} // end inner while

				// 将构造出的 outPoly 作为一个结果，如果有效则加入 results
				if (outPoly.size() >= 3)
				{
					// 去重：首尾相等时去掉尾
					if (IsSamePoint(outPoly.front(), outPoly.back())) outPoly.pop_back();
					if (outPoly.size() >= 3) results.push_back(outPoly);
				}
			} // end for each inters startIdx

			// 如果通过交点遍历没有得到任何环（可能因为实现路径未覆盖某些情况），使用退路：Sutherland–Hodgman
			if (results.empty())
			{
				auto fallback = SutherlandHodgmanClip(subject, left_top, right_bottom);
				if (!fallback.empty()) results.push_back(fallback);
			}

			return results;
		}

	} // namespace Clip
} // namespace GraphicFunc
