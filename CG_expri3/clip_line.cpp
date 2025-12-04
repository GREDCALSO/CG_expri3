#include"clip_line.h"

namespace GraphicFunc
{
	namespace Clip
	{
		int ComputeOutCode(int x, int y, const POINT& left_top, const POINT& right_bottom)
		{
			int code = 0;	//默认矩形内部

			if (x < left_top.x)
			{
				//在矩形左边
				code |= 1;	//和 0001 逻辑或
			}
			else if (x > right_bottom.x)
			{
				//在矩形右边
				code |= 2;	//和 0010 逻辑或
			}

			if (y < left_top.y)
			{
				//在矩形上边
				code |= 8;	//和 1000 逻辑或
			}
			else if (y > right_bottom.y)
			{
				//在矩形下边
				code |= 4;	//和 0100 逻辑或
			}

			return code;
		}

		bool clip_CohenSutherland(int& x0, int& y0, int& x1, int& y1, const POINT& left_top, const POINT& right_bottom)
		{
			int code0 = ComputeOutCode(x0, y0, left_top, right_bottom);
			int code1 = ComputeOutCode(x1, y1, left_top, right_bottom);

			while (true)
			{
				//完全在窗口内
				if ((code0 | code1) == 0) { return true; }
				//完全在窗口外
				else if (code0 & code1) { return false; }

				//部分在窗口内，那么终点必然在窗口外
				//起点在窗口外：outcode = code0；起点在窗口内：outcode = code1
				int outcode = code0 ? code0 : code1;

				float x, y;

				float dx = (float)(x1 - x0);
				float dy = (float)(y1 - y0);

				if (outcode & 8)
				{
					y = (float)left_top.y;
					x = x0 + dx * (y - y0) / dy;
				}
				else if (outcode & 4)
				{
					y = (float)right_bottom.y;
					x = x0 + dx * (y - y0) / dy;
				}
				else if (outcode & 2)
				{
					x = (float)right_bottom.x;
					y = y0 + dy * (x - x0) / dx;
				}
				else if (outcode & 1)
				{
					x = (float)left_top.x;
					y = y0 + dy * (x - x0) / dx;
				}

				if (outcode == code0)
				{
					//起点在窗口外，将被裁出的交点作为新起点
					x0 = (int)x;
					y0 = (int)y;
					code0 = ComputeOutCode(x0, y0, left_top, right_bottom);
				}
				else
				{
					//起点在窗口内，将被裁出的交点作为新终点
					x1 = (int)x;
					y1 = (int)y;
					code1 = ComputeOutCode(x1, y1, left_top, right_bottom);
				}
			}
		}

		bool clip_MidpointSubdivision(int& x0, int& y0, int& x1, int& y1, const POINT& left_top, const POINT& right_bottom)
		{
			//判断线段是否完全在窗口内
			auto LineInside = [&](int x0, int y0, int x1, int y1, const POINT& left_top, const POINT& right_bottom)
				{
					return ComputeOutCode(x0, y0, left_top, right_bottom) == 0
						&& ComputeOutCode(x1, y1, left_top, right_bottom) == 0;
				};

			//判断线段是否完全在窗口外
			auto LineOutside = [&](int x0, int y0, int x1, int y1, const POINT& left_top, const POINT& right_bottom)
				{
					int code0 = ComputeOutCode(x0, y0, left_top, right_bottom);
					int code1 = ComputeOutCode(x1, y1, left_top, right_bottom);
					return (code0 & code1) != 0;
				};

			// 如果线段完全在内部，直接返回
			if (LineInside(x0, y0, x1, y1, left_top, right_bottom))
			{
				return true;
			}

			// 如果线段完全在外部，直接返回
			if (LineOutside(x0, y0, x1, y1, left_top, right_bottom))
			{
				return false;
			}

			// 中点分割法 - 迭代版本
			int x_start = x0, y_start = y0;
			int x_end = x1, y_end = y1;

			// 处理起点在窗口外的情况
			int code_start = ComputeOutCode(x_start, y_start, left_top, right_bottom);
			while (code_start != 0)
			{
				int x_mid = (x_start + x_end) / 2;
				int y_mid = (y_start + y_end) / 2;

				// 如果中点在窗口内，替换起点
				int code_mid = ComputeOutCode(x_mid, y_mid, left_top, right_bottom);
				if (code_mid == 0)
				{
					x_start = x_mid;
					y_start = y_mid;
					code_start = 0;
				}
				// 如果中点也在窗口外，检查是否与起点在同一侧
				else if (code_mid & code_start)
				{
					x_start = x_mid;
					y_start = y_mid;
					code_start = code_mid;
				}
				// 否则中点和起点在不同侧，替换终点
				else
				{
					x_end = x_mid;
					y_end = y_mid;
				}

				// 如果线段足够短，终止迭代
				if (abs(x_start - x_end) <= 1 && abs(y_start - y_end) <= 1)
				{
					break;
				}
			}

			// 处理终点在窗口外的情况
			int code_end = ComputeOutCode(x_end, y_end, left_top, right_bottom);
			while (code_end != 0)
			{
				int x_mid = (x_start + x_end) / 2;
				int y_mid = (y_start + y_end) / 2;

				// 如果中点在窗口内，替换终点
				int code_mid = ComputeOutCode(x_mid, y_mid, left_top, right_bottom);
				if (code_mid == 0)
				{
					x_end = x_mid;
					y_end = y_mid;
					code_end = 0;
				}
				// 如果中点也在窗口外，检查是否与终点在同一侧
				else if (code_mid & code_end)
				{
					x_end = x_mid;
					y_end = y_mid;
					code_end = code_mid;
				}
				// 否则中点和终点在不同侧，替换起点
				else
				{
					x_start = x_mid;
					y_start = y_mid;
				}

				// 如果线段足够短，终止迭代
				if (abs(x_start - x_end) <= 1 && abs(y_start - y_end) <= 1)
				{
					break;
				}
			}

			// 更新原始坐标
			x0 = x_start;
			y0 = y_start;
			x1 = x_end;
			y1 = y_end;

			// 最后检查线段是否在窗口内
			return LineInside(x0, y0, x1, y1, left_top, right_bottom);
		}
	}
}