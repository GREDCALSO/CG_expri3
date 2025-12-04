#include"clip_polygon.h"
#include<algorithm>
#include<Windows.h>
#include<cmath>
#include<limits>
#include<cassert>

namespace GraphicFunc
{
	namespace Clip
	{
		static const double EPS = 1e-6;

		// ---------------------- 基本几何工具 ----------------------
		struct PointF { double x, y; };
		static PointF toF(const POINT& p) { return { (double)p.x, (double)p.y }; }
		static POINT toI(const PointF& p) { return { (LONG)std::round(p.x), (LONG)std::round(p.y) }; }

		static double cross(double ax, double ay, double bx, double by) { return ax * by - ay * bx; }

		static bool segSegIntersect(const PointF& p1, const PointF& p2, const PointF& q1, const PointF& q2, PointF& out, double* outT = nullptr, double* outU = nullptr)
		{
			double r_x = p2.x - p1.x, r_y = p2.y - p1.y;
			double s_x = q2.x - q1.x, s_y = q2.y - q1.y;
			double denom = cross(r_x, r_y, s_x, s_y);
			double qp_x = q1.x - p1.x, qp_y = q1.y - p1.y;

			if (fabs(denom) < EPS) return false;

			double t = cross(qp_x, qp_y, s_x, s_y) / denom;
			double u = cross(qp_x, qp_y, r_x, r_y) / denom;

			if (t >= -EPS && t <= 1.0 + EPS && u >= -EPS && u <= 1.0 + EPS)
			{
				out.x = p1.x + t * r_x;
				out.y = p1.y + t * r_y;
				if (outT) *outT = t;
				if (outU) *outU = u;
				return true;
			}
			return false;
		}

		static bool isLeftOrOn(const PointF& a, const PointF& b, const PointF& r)
		{
			return cross(b.x - a.x, b.y - a.y, r.x - a.x, r.y - a.y) >= -EPS;
		}

		// ---------------------- Sutherland-Hodgman ----------------------
		static std::vector<PointF> clipAgainstEdge(const std::vector<PointF>& input, const PointF& clipA, const PointF& clipB)
		{
			std::vector<PointF> out;
			if (input.empty()) return out;

			for (size_t i = 0, n = input.size(); i < n; ++i)
			{
				PointF cur = input[i];
				PointF prev = input[(i + n - 1) % n];
				bool curInside = isLeftOrOn(clipA, clipB, cur);
				bool prevInside = isLeftOrOn(clipA, clipB, prev);

				if (curInside)
				{
					if (!prevInside)
					{
						PointF ip;
						if (segSegIntersect(prev, cur, clipA, clipB, ip))
							out.push_back(ip);
					}
					out.push_back(cur);
				}
				else if (prevInside)
				{
					PointF ip;
					if (segSegIntersect(prev, cur, clipA, clipB, ip))
						out.push_back(ip);
				}
			}
			return out;
		}

		std::vector<POINT> clip_SutherlandHodgman(const std::vector<POINT>& subject, const RECT& clipRect)
		{
			std::vector<PointF> input;
			input.reserve(subject.size());
			for (auto& p : subject) input.push_back(toF(p));
			if (input.empty()) return {};

			PointF bl { (double)clipRect.left, (double)clipRect.bottom };
			PointF tl { (double)clipRect.left, (double)clipRect.top };
			PointF tr { (double)clipRect.right, (double)clipRect.top };
			PointF br { (double)clipRect.right, (double)clipRect.bottom };

			std::vector<std::pair<PointF, PointF>> clipEdges = {
				{ tl, tr }, { tr, br }, { br, bl }, { bl, tl }
			};

			std::vector<PointF> cur = input;
			for (auto& e : clipEdges)
			{
				cur = clipAgainstEdge(cur, e.first, e.second);
				if (cur.empty()) break;
			}

			std::vector<POINT> result;
			result.reserve(cur.size());
			for (auto& pf : cur) result.push_back(toI(pf));
			return result;
		}

		// 射线法判断点在多边形内
		static bool pointInPoly(const PointF& p, const std::vector<PointF>& poly)
		{
			bool inside = false;
			size_t n = poly.size();
			for (size_t i = 0, j = n - 1; i < n; j = i++)
			{
				double xi = poly[i].x, yi = poly[i].y;
				double xj = poly[j].x, yj = poly[j].y;
				if (((yi > p.y) != (yj > p.y)) &&
					(p.x < (xj - xi) * (p.y - yi) / (yj - yi + 1e-12) + xi))
					inside = !inside;
			}
			return inside;
		}

		// 计算多边形面积
		static double polygonArea(const std::vector<PointF>& poly)
		{
			double area = 0.0;
			size_t n = poly.size();
			for (size_t i = 0; i < n; ++i)
			{
				size_t j = (i + 1) % n;
				area += poly[i].x * poly[j].y - poly[j].x * poly[i].y;
			}
			return area * 0.5;
		}

		// ===================== Weiler-Atherton (数组实现) =====================
		std::vector<POINT> clip_WeilerAtherton(const std::vector<POINT>& subject, const std::vector<POINT>& clipPoly)
		{
			if (subject.size() < 3 || clipPoly.size() < 3) return {};

			// 转换为浮点
			std::vector<PointF> subjF, clipF;
			for (auto& p : subject) subjF.push_back(toF(p));
			for (auto& p : clipPoly) clipF.push_back(toF(p));

			// 确保逆时针
			if (polygonArea(subjF) < 0) std::reverse(subjF.begin(), subjF.end());
			if (polygonArea(clipF) < 0) std::reverse(clipF.begin(), clipF.end());

			// 顶点结构
			struct Vertex
			{
				PointF pt;
				bool isInter = false;
				bool isEntry = false;
				bool visited = false;
				int nextIdx = -1;      // 同一多边形中的下一个顶点
				int neighborIdx = -1;  // 配对交点在另一多边形中的索引
			};

			std::vector<Vertex> subjList, clipList;

			// 初始化主多边形顶点
			for (size_t i = 0; i < subjF.size(); ++i)
			{
				Vertex v;
				v.pt = subjF[i];
				v.nextIdx = (int)((i + 1) % subjF.size());
				subjList.push_back(v);
			}

			// 初始化裁剪多边形顶点
			for (size_t i = 0; i < clipF.size(); ++i)
			{
				Vertex v;
				v.pt = clipF[i];
				v.nextIdx = (int)((i + 1) % clipF.size());
				clipList.push_back(v);
			}

			// 找所有交点
			struct InterInfo
			{
				PointF pt;
				int subjEdge;
				double tSubj;
				int clipEdge;
				double tClip;
			};
			std::vector<InterInfo> allInters;

			for (size_t i = 0; i < subjF.size(); ++i)
			{
				PointF s1 = subjF[i];
				PointF s2 = subjF[(i + 1) % subjF.size()];

				for (size_t j = 0; j < clipF.size(); ++j)
				{
					PointF c1 = clipF[j];
					PointF c2 = clipF[(j + 1) % clipF.size()];

					PointF ip;
					double t, u;
					if (segSegIntersect(s1, s2, c1, c2, ip, &t, &u))
					{
						if (t > EPS && t < 1.0 - EPS && u > EPS && u < 1.0 - EPS)
						{
							allInters.push_back({ ip, (int)i, t, (int)j, u });
						}
					}
				}
			}

			// 无交点情况
			if (allInters.empty())
			{
				if (pointInPoly(subjF[0], clipF)) return subject;
				if (pointInPoly(clipF[0], subjF)) return clipPoly;
				return {};
			}

			// 按边和参数排序，插入交点到两个多边形
			// 为每条主多边形边收集交点
			for (size_t i = 0; i < subjF.size(); ++i)
			{
				std::vector<std::pair<double, int>> edgeInters;
				for (size_t k = 0; k < allInters.size(); ++k)
				{
					if (allInters[k].subjEdge == (int)i)
					{
						edgeInters.push_back({ allInters[k].tSubj, (int)k });
					}
				}

				if (edgeInters.empty()) continue;

				std::sort(edgeInters.begin(), edgeInters.end());

				// 找到边的起点在 subjList 中的位置
				int startIdx = (int)i;
				int endIdx = subjList[startIdx].nextIdx;

				int prevIdx = startIdx;
				for (auto& pr : edgeInters)
				{
					Vertex v;
					v.pt = allInters[pr.second].pt;
					v.isInter = true;
					v.nextIdx = endIdx;

					int newIdx = (int)subjList.size();
					subjList.push_back(v);

					subjList[prevIdx].nextIdx = newIdx;
					prevIdx = newIdx;

					// 记录该交点在 subjList 中的索引
					allInters[pr.second].subjEdge = newIdx; // 复用字段存储新索引
				}
			}

			// 为每条裁剪多边形边收集交点
			for (size_t j = 0; j < clipF.size(); ++j)
			{
				std::vector<std::pair<double, int>> edgeInters;
				for (size_t k = 0; k < allInters.size(); ++k)
				{
					if (allInters[k].clipEdge == (int)j)
					{
						edgeInters.push_back({ allInters[k].tClip, (int)k });
					}
				}

				if (edgeInters.empty()) continue;

				std::sort(edgeInters.begin(), edgeInters.end());

				int startIdx = (int)j;
				int endIdx = clipList[startIdx].nextIdx;

				int prevIdx = startIdx;
				for (auto& pr : edgeInters)
				{
					Vertex v;
					v.pt = allInters[pr.second].pt;
					v.isInter = true;
					v.nextIdx = endIdx;

					int newIdx = (int)clipList.size();
					clipList.push_back(v);

					clipList[prevIdx].nextIdx = newIdx;
					prevIdx = newIdx;

					// 建立配对关系
					int subjIdx = allInters[pr.second].subjEdge;
					subjList[subjIdx].neighborIdx = newIdx;
					clipList[newIdx].neighborIdx = subjIdx;
				}
			}

			// 标记进入/退出
			for (size_t i = 0; i < subjList.size(); ++i)
			{
				if (!subjList[i].isInter) continue;

				int nextIdx = subjList[i].nextIdx;
				PointF cur = subjList[i].pt;
				PointF next = subjList[nextIdx].pt;

				double dx = next.x - cur.x;
				double dy = next.y - cur.y;
				double dist = std::sqrt(dx * dx + dy * dy);

				if (dist > EPS)
				{
					double step = min(0.01, dist * 0.1);
					PointF probe { cur.x + dx / dist * step, cur.y + dy / dist * step };
					subjList[i].isEntry = pointInPoly(probe, clipF);

					// 裁剪多边形中的配对点标记相反
					int nb = subjList[i].neighborIdx;
					if (nb >= 0 && nb < (int)clipList.size())
					{
						clipList[nb].isEntry = !subjList[i].isEntry;
					}
				}
			}

			// 遍历生成结果多边形
			std::vector<std::vector<PointF>> results;

			for (size_t startIdx = 0; startIdx < subjList.size(); ++startIdx)
			{
				if (!subjList[startIdx].isInter) continue;
				if (subjList[startIdx].visited) continue;
				if (!subjList[startIdx].isEntry) continue;

				std::vector<PointF> poly;
				int curIdx = (int)startIdx;
				bool onSubj = true;
				int maxIter = (int)(subjList.size() + clipList.size()) * 2;

				for (int iter = 0; iter < maxIter; ++iter)
				{
					if (onSubj)
					{
						subjList[curIdx].visited = true;
						poly.push_back(subjList[curIdx].pt);

						if (subjList[curIdx].isInter && curIdx != (int)startIdx)
						{
							// 切换到裁剪多边形
							int nb = subjList[curIdx].neighborIdx;
							if (nb < 0) break;
							curIdx = nb;
							onSubj = false;
						}
						else
						{
							curIdx = subjList[curIdx].nextIdx;
						}
					}
					else
					{
						clipList[curIdx].visited = true;
						poly.push_back(clipList[curIdx].pt);

						if (clipList[curIdx].isInter)
						{
							// 切换回主多边形
							int nb = clipList[curIdx].neighborIdx;
							if (nb < 0) break;
							curIdx = nb;
							onSubj = true;

							if (curIdx == (int)startIdx) break;
						}
						else
						{
							curIdx = clipList[curIdx].nextIdx;
						}
					}

					if (onSubj && curIdx == (int)startIdx) break;
				}

				// 去重
				if (poly.size() >= 3)
				{
					std::vector<PointF> clean;
					for (auto& p : poly)
					{
						if (clean.empty() ||
							std::fabs(clean.back().x - p.x) > EPS ||
							std::fabs(clean.back().y - p.y) > EPS)
						{
							clean.push_back(p);
						}
					}
					if (clean.size() >= 3)
					{
						if (std::fabs(clean.front().x - clean.back().x) < EPS &&
							std::fabs(clean.front().y - clean.back().y) < EPS)
						{
							clean.pop_back();
						}
						if (clean.size() >= 3)
							results.push_back(clean);
					}
				}
			}

			if (results.empty())
			{
				if (pointInPoly(subjF[0], clipF)) return subject;
				return {};
			}

			// 返回顶点最多的多边形
			size_t bestIdx = 0;
			for (size_t i = 1; i < results.size(); ++i)
			{
				if (results[i].size() > results[bestIdx].size())
					bestIdx = i;
			}

			std::vector<POINT> result;
			for (auto& pf : results[bestIdx])
				result.push_back(toI(pf));

			return result;
		}
	}
}