#pragma once
#include<vector>
#include<Windows.h>

namespace GraphicFunc
{
	namespace Clip
	{
		std::vector<POINT> clip_SutherlandHodgman(const std::vector<POINT>& subject, const RECT& clipRect);
		std::vector<POINT> clip_WeilerAtherton(const std::vector<POINT>& subject, const std::vector<POINT>& clipPoly);
	}
}