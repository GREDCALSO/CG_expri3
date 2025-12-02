#include"utils.h"

namespace
{
	// 点到线段距离
	double DistancePointToSegment(POINT p, POINT a, POINT b) {
		double vx = static_cast<double>(b.x - a.x);
		double vy = static_cast<double>(b.y - a.y);
		double wx = static_cast<double>(p.x - a.x);
		double wy = static_cast<double>(p.y - a.y);

		double len2 = vx * vx + vy * vy;
		if (len2 <= 1e-12)
		{
			// 退化为点
			double dx = static_cast<double>(p.x - a.x);
			double dy = static_cast<double>(p.y - a.y);
			return std::sqrt(dx * dx + dy * dy);
		}

		double t = (wx * vx + wy * vy) / len2;
		if (t < 0.0) t = 0.0;
		else if (t > 1.0) t = 1.0;

		double projx = a.x + t * vx;
		double projy = a.y + t * vy;

		double dx = static_cast<double>(p.x) - projx;
		double dy = static_cast<double>(p.y) - projy;
		return std::sqrt(dx * dx + dy * dy);
	}

	// 点在多边形内（射线法），含边界判定（通过阈值近似）
	bool PointInPolygon(const POINT& p, const std::vector<POINT>& poly, double edgeTol) {
		const size_t n = poly.size();
		if (n < 3) return false;

		// 边界近似命中（点击靠近边则认为命中）
		for (size_t i = 0, j = n - 1; i < n; j = i++)
		{
			if (DistancePointToSegment(p, poly[j], poly[i]) <= edgeTol)
			{
				return true;
			}
		}

		bool inside = false;
		for (size_t i = 0, j = n - 1; i < n; j = i++)
		{
			const LONG xi = poly[i].x, yi = poly[i].y;
			const LONG xj = poly[j].x, yj = poly[j].y;

			// 判断与水平射线是否相交
			const bool intersect = ((yi > p.y) != (yj > p.y)) &&
				(p.x < (static_cast<double>(xj - xi) * (p.y - yi)) / (static_cast<double>(yj - yi)) + xi);

			if (intersect) inside = !inside;
		}
		return inside;
	}
}

bool PointInsideShape(POINT mousePos, Shape seletedShape)
{
	constexpr double kHitTolerance = 5.0;

	switch (seletedShape.type)
	{
	case ShapeType::Line:
	{
		if (seletedShape.vertices.size() < 2) return false;
		POINT a = seletedShape.vertices[0];
		POINT b = seletedShape.vertices[1];
		return DistancePointToSegment(mousePos, a, b) <= kHitTolerance;
	}
	case ShapeType::Polygon:
	{
		return PointInPolygon(mousePos, seletedShape.vertices, kHitTolerance);
	}
	default:
		return false;
	}
}

void AddShape_line(std::vector<Shape>& gShape, POINT start, POINT end)
{
	Shape sp;
	sp.type = ShapeType::Line;
	sp.vertices.clear();
	sp.vertices.push_back(start);
	sp.vertices.push_back(end);

	gShape.push_back(sp);
}