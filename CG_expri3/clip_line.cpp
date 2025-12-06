#include"clip_line.h"
#include<cmath>

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
			// 初始编码
			int code0 = ComputeOutCode(x0, y0, left_top, right_bottom);
			int code1 = ComputeOutCode(x1, y1, left_top, right_bottom);
			
			// 完全在窗口外（两端点在窗口同一侧外）
			if (code0 & code1) {
				return false;
			}
			
			// 完全在窗口内
			if (code0 == 0 && code1 == 0) {
				return true;
			}

			// 使用浮点数以提高精度
			double px0 = x0, py0 = y0;
			double px1 = x1, py1 = y1;
			
			const int MAX_ITERATIONS = 30;
			const double PRECISION = 0.5;

			// 步骤1: 如果起点在窗口外，找到从起点方向的第一个进入点
			if (code0 != 0) {
				double a_x = px0, a_y = py0;  // 外部点
				double b_x = px1, b_y = py1;  // 另一个点
				
				for (int iter = 0; iter < MAX_ITERATIONS; iter++) {
					double mid_x = (a_x + b_x) / 2.0;
					double mid_y = (a_y + b_y) / 2.0;
					
					int code_mid = ComputeOutCode((int)round(mid_x), (int)round(mid_y), left_top, right_bottom);
					int code_a = ComputeOutCode((int)round(a_x), (int)round(a_y), left_top, right_bottom);
					
					// 如果中点在内部，交点在 a 和 mid 之间
					if (code_mid == 0) {
						b_x = mid_x;
						b_y = mid_y;
					}
					// 如果中点和 a 在同一区域外，交点在 mid 和 b 之间
					else if (code_mid & code_a) {
						a_x = mid_x;
						a_y = mid_y;
					}
					// 中点在不同的外部区域，交点在 a 和 mid 之间
					else {
						b_x = mid_x;
						b_y = mid_y;
					}
					
					// 检查精度
					double dist = sqrt((b_x - a_x) * (b_x - a_x) + (b_y - a_y) * (b_y - a_y));
					if (dist < PRECISION) {
						break;
					}
				}
				
				// 取更接近窗口内部的点
				int code_a = ComputeOutCode((int)round(a_x), (int)round(a_y), left_top, right_bottom);
				int code_b = ComputeOutCode((int)round(b_x), (int)round(b_y), left_top, right_bottom);
				
				if (code_b == 0) {
					px0 = b_x;
					py0 = b_y;
				} else if (code_a == 0) {
					px0 = a_x;
					py0 = a_y;
				} else {
					// 都不在内部，取中点
					px0 = (a_x + b_x) / 2.0;
					py0 = (a_y + b_y) / 2.0;
				}
			}

			// 步骤2: 如果终点在窗口外，找到从终点方向的第一个进入点
			code1 = ComputeOutCode(x1, y1, left_top, right_bottom);
			if (code1 != 0) {
				double a_x = px1, a_y = py1;  // 外部点
				double b_x = px0, b_y = py0;  // 另一个点（已更新的起点）
				
				for (int iter = 0; iter < MAX_ITERATIONS; iter++) {
					double mid_x = (a_x + b_x) / 2.0;
					double mid_y = (a_y + b_y) / 2.0;
					
					int code_mid = ComputeOutCode((int)round(mid_x), (int)round(mid_y), left_top, right_bottom);
					int code_a = ComputeOutCode((int)round(a_x), (int)round(a_y), left_top, right_bottom);
					
					// 如果中点在内部，交点在 a 和 mid 之间
					if (code_mid == 0) {
						b_x = mid_x;
						b_y = mid_y;
					}
					// 如果中点和 a 在同一区域外，交点在 mid 和 b 之间
					else if (code_mid & code_a) {
						a_x = mid_x;
						a_y = mid_y;
					}
					// 中点在不同的外部区域，交点在 a 和 mid 之间
					else {
						b_x = mid_x;
						b_y = mid_y;
					}
					
					// 检查精度
					double dist = sqrt((b_x - a_x) * (b_x - a_x) + (b_y - a_y) * (b_y - a_y));
					if (dist < PRECISION) {
						break;
					}
				}
				
				// 取更接近窗口内部的点
				int code_a = ComputeOutCode((int)round(a_x), (int)round(a_y), left_top, right_bottom);
				int code_b = ComputeOutCode((int)round(b_x), (int)round(b_y), left_top, right_bottom);
				
				if (code_b == 0) {
					px1 = b_x;
					py1 = b_y;
				} else if (code_a == 0) {
					px1 = a_x;
					py1 = a_y;
				} else {
					// 都不在内部，取中点
					px1 = (a_x + b_x) / 2.0;
					py1 = (a_y + b_y) / 2.0;
				}
			}

			// 更新坐标（使用round四舍五入）
			x0 = (int)round(px0);
			y0 = (int)round(py0);
			x1 = (int)round(px1);
			y1 = (int)round(py1);

			// 最终验证
			code0 = ComputeOutCode(x0, y0, left_top, right_bottom);
			code1 = ComputeOutCode(x1, y1, left_top, right_bottom);
			
			// 只要两个端点都在窗口内或边界上就成功
			return (code0 == 0 && code1 == 0);
		}
	}
}