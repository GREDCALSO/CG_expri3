#pragma once

enum class GraphicMode
{
	None,
	LineBresenham,
	LineMidpoint,
	CircleBresenham,
	CircleMidpoint,
	Rectangle,
	Brush,
	FillScanLine,
	FillFence,
	BSpline,
	RandomPolygon,
	RandomCurve,
	Translate,
	Scale,
	Rotate,
	ClipLine_CohenSutherland,
	ClipLine_MidpointSubdivision,
	ClipPolygon_SutherlandHodgman,
	ClipPolygon_WeilerAtherton
};