#pragma once

#include<Windows.h>

namespace GraphicFunc
{
	namespace Clip
	{
		//º∆À„∂Àµ„±‡¬Î
		int ComputeOutCode(int x, int y, const POINT& left_top, const POINT& right_bottom);

		bool clip_CohenSutherland(int& x0, int& y0, int& x1, int& y1, const POINT& left_top, const POINT& right_bottom);

		bool clip_MidpointSubdivision(int& x0, int& y0, int& x1, int& y1, const POINT& left_top, const POINT& right_bottom);
	}
}