#pragma once
#include<Windows.h>
#include<vector>
#include<cmath>
#include"component_shape.h"

//检查鼠标点击位置是否选中图形
bool PointInsideShape(POINT mousePos, Shape seletedShape);

void AddShape_line(std::vector<Shape>& gShape, POINT start, POINT end);