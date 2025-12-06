#include<Windows.h>
#include<d2d1.h>
#include"graphic_algorithms.h"
#include"graphic_modes.h"
#include"renderer.h"
#include"ui.h"
#include"component_shape.h"
#include"utils.h"

Renderer renderer;
POINT pos_start = { 0 };
POINT pos_end = { 0 };
std::vector<POINT> control_points;
bool is_drawing = false;

std::vector<Shape> gShapes;	//全局图元
int selectedShape = -1;	//当前选中的图形
POINT pivot_point = { 0,0 };	//旋转中心
bool hasPivotPoint = false;	//是否设置了旋转中心
Shape originalShape;	//保存选中图像的原状态，预览时调用
bool hasOriginalShape = false;

GraphicMode graphic_modes = GraphicMode::LineBresenham;

HWND main_hwnd = nullptr;

//重绘图形
void RedrawAllShapes(HWND hwnd)
{
	renderer.Clear();
	for (auto& s : gShapes)
	{
		if (s.type == ShapeType::Line && s.vertices.size() >= 2)
		{
			GraphicFunc::Line::DrawLineBresenham(
				s.vertices[0].x, s.vertices[0].y,
				s.vertices[1].x, s.vertices[1].y,
				renderer
			);
		}
		else if (s.type == ShapeType::Polygon && s.vertices.size() >= 2)
		{
			GraphicFunc::Polygon::DrawRandomPolygon(s.vertices, renderer);
		}
	}

	//如果在旋转模式且设置了旋转中心，重新绘制旋转中心点
	if (graphic_modes == GraphicMode::Rotate && hasPivotPoint)
	{
		GraphicFunc::MarkPoint::DrawMarkPoint(pivot_point.x, pivot_point.y, renderer);
	}

	InvalidateRect(hwnd, NULL, FALSE);
}

LRESULT CALLBACK WinProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_COMMAND:
	{
		const int id = LOWORD(wParam);
		const int code = HIWORD(wParam);

		//按钮事件
		if (id == ID_BTN_CLEANCANVAS && code == BN_CLICKED)
		{
			//清屏
			renderer.Clear();
			control_points.clear();	//清空控制点
			gShapes.clear();	//清空图元表
			selectedShape = -1;	//重置选中图形
			hasPivotPoint = false;	//清除旋转中心
			InvalidateRect(hwnd, nullptr, FALSE);
			return 0;
		}

		if (id == ID_CB_SELECTGRAPHICS && code == CBN_SELCHANGE)
		{
			int sel = SendMessage(combo_SelectGraphics, CB_GETCURSEL, 0, 0);
			if (sel == CB_ERR)
			{
				MessageBoxW(hwnd, L"错误提示", L"下拉框有bug", MB_OK | MB_ICONERROR);
			}

			GraphicMode oldMode = graphic_modes;	//记录先前的模式

			switch (sel)
			{
			case 0:
				graphic_modes = GraphicMode::LineBresenham;
				SetWindowTextW(label_OperationMode, L"按下鼠标左键位置为线段起点，释放鼠标左键位置为线段终点");
				break;

			case 1:
				graphic_modes = GraphicMode::LineMidpoint;
				SetWindowTextW(label_OperationMode, L"按下鼠标左键位置为线段起点，释放鼠标左键位置为线段终点");
				break;

			case 2:
				graphic_modes = GraphicMode::CircleBresenham;
				SetWindowTextW(label_OperationMode, L"按下鼠标左键位置为直径起点，释放鼠标左键位置为直径终点");
				break;

			case 3:
				graphic_modes = GraphicMode::CircleMidpoint;
				SetWindowTextW(label_OperationMode, L"按下鼠标左键位置为直径起点，释放鼠标左键位置为直径终点");
				break;

			case 4:
				graphic_modes = GraphicMode::Rectangle;
				SetWindowTextW(label_OperationMode, L"按下鼠标左键位置为对角线起点，释放鼠标左键位置为对角线终点");
				break;

			case 5:
				graphic_modes = GraphicMode::Brush;
				SetWindowTextW(label_OperationMode, L"按住鼠标左键即可绘制");
				break;

			case 6:
				graphic_modes = GraphicMode::BSpline;
				SetWindowTextW(label_OperationMode, L"按下鼠标右键位置添加控制点，点击鼠标左键抬起时即可绘制，小于4个控制点无法绘制");
				break;

			case 7:
				graphic_modes = GraphicMode::FillScanLine;
				SetWindowTextW(label_OperationMode, L"按下鼠标右键位置添加控制点，点击鼠标左键抬起时即可绘制，小于3个控制点无法绘制");
				break;

			case 8:
				graphic_modes = GraphicMode::FillFence;
				SetWindowTextW(label_OperationMode, L"按下鼠标右键位置添加控制点，点击鼠标左键抬起时即可绘制，小于3个控制点无法绘制");
				break;

			case 9:
				graphic_modes = GraphicMode::RandomPolygon;
				SetWindowTextW(label_OperationMode, L"按下鼠标右键位置添加控制点，点击鼠标左键抬起时即可绘制，小于2个控制点无法绘制");
				break;

			case 10:
				graphic_modes = GraphicMode::RandomCurve;
				SetWindowTextW(label_OperationMode, L"圆弧（用了D2D库）");
				break;

			case 11:
				graphic_modes = GraphicMode::Translate;
				SetWindowTextW(label_OperationMode, L"点击选中图形，按住拖动平移");
				break;

			case 12:
				graphic_modes = GraphicMode::Scale;
				SetWindowTextW(label_OperationMode, L"点击选中图形，按住拖拽缩放（向右下放大，向左上缩小）");
				break;

			case 13:
				graphic_modes = GraphicMode::Rotate;
				SetWindowTextW(label_OperationMode, L"右键设置旋转中心，左键点击选中图形，按住拖拽旋转（向右顺时针旋转，向左逆时针旋转）");
				break;

			case 14:
				graphic_modes = GraphicMode::ClipLine_CohenSutherland;
				SetWindowTextW(label_OperationMode, L"按下鼠标左键位置为对角线起点，释放鼠标左键位置为对角线终点，左键抬起完成裁切");
				break;

			case 15:
				graphic_modes = GraphicMode::ClipLine_MidpointSubdivision;
				SetWindowTextW(label_OperationMode, L"按下鼠标左键位置为对角线起点，释放鼠标左键位置为对角线终点，左键抬起完成裁切");
				break;

			case 16:
				graphic_modes = GraphicMode::ClipPolygon_SutherlandHodgman;
				SetWindowTextW(label_OperationMode, L"按下鼠标左键位置为对角线起点，释放鼠标左键位置为对角线终点，左键抬起完成裁切");
				break;

			case 17:
				graphic_modes = GraphicMode::ClipPolygon_WeilerAtherton;
				SetWindowTextW(label_OperationMode, L"按下鼠标左键位置为对角线起点，释放鼠标左键位置为对角线终点，左键抬起完成裁切");
				break;

			default:
				graphic_modes = GraphicMode::LineBresenham;
				SetWindowTextW(label_OperationMode, L"按下鼠标左键位置为线段起点，释放鼠标左键位置为线段终点");
				break;
			}

			//如果从旋转模式切换到其他模式，需要清理旋转中心
			if (oldMode == GraphicMode::Rotate && graphic_modes != GraphicMode::Rotate)
			{
				hasPivotPoint = false;
				RedrawAllShapes(hwnd);
			}
		}

		return 0;
	}

	case WM_LBUTTONDOWN:
	{
		int x = LOWORD(lParam);
		int y = HIWORD(lParam);
		pos_start.x = x;
		pos_start.y = y;
		is_drawing = true;

		//画笔模式，线段连续，初始化上一点
		if (graphic_modes == GraphicMode::Brush)
		{
			pos_end = pos_start;
		}

		//Transform选中图形
		if (graphic_modes == GraphicMode::Translate ||
			graphic_modes == GraphicMode::Scale ||
			graphic_modes == GraphicMode::Rotate)
		{
			selectedShape = -1; //先重置选中
			for (int i = (int)gShapes.size() - 1; i >= 0; i--) // 从上层往下找
			{
				if (PointInsideShape({ x, y }, gShapes[i]))
				{
					selectedShape = i;
					//保存原始图形状态
					originalShape = gShapes[i];
					hasOriginalShape = true;
					break;
				}
			}
		}

		return 0;
	}

	case WM_RBUTTONDOWN:
	{
		int x = LOWORD(lParam);
		int y = HIWORD(lParam);

		//在选择模式下右键设置旋转中心
		if (graphic_modes == GraphicMode::Rotate)
		{
			pivot_point.x = x;
			pivot_point.y = y;
			hasPivotPoint = true;
			//绘制一个标记点表示旋转中心
			GraphicFunc::MarkPoint::DrawMarkPoint(x, y, renderer);
			InvalidateRect(hwnd, NULL, FALSE);
			return 0;
		}

		//有 4 种情况需要控制点
		switch (graphic_modes)
		{
		case GraphicMode::FillScanLine:
		case GraphicMode::FillFence:
		case GraphicMode::BSpline:
		case GraphicMode::RandomPolygon:
			control_points.push_back(POINT { x, y });
			GraphicFunc::MarkPoint::DrawMarkPoint(x, y, renderer);
			break;
		}

		return 0;
	}

	case WM_MOUSEMOVE:
	{
		int x = LOWORD(lParam);
		int y = HIWORD(lParam);

		switch (graphic_modes)
		{
		case GraphicMode::Brush:
			if (is_drawing)
			{
				if (x != pos_end.x || y != pos_end.y)
				{
					GraphicFunc::Brush::DrawFreeLine(pos_end.x, pos_end.y, x, y, renderer);
					pos_end.x = x;
					pos_end.y = y;
				}
			}
			break;

		case GraphicMode::Translate:
			//实时预览平移
			if (is_drawing && selectedShape >= 0 && selectedShape < (int)gShapes.size() && hasOriginalShape)
			{
				// 计算相对于初始点击位置的偏移量
				int dx = x - pos_start.x;
				int dy = y - pos_start.y;

				// 基于原始图形进行变换
				Shape tempShape = originalShape;
				float M[3][3];
				GraphicFunc::Transform::GetTranslateMatrix((float)dx, (float)dy, M);
				GraphicFunc::Transform::TransformShape(tempShape.vertices, M);

				// 更新选中图形为预览状态
				gShapes[selectedShape] = tempShape;

				// 重绘所有图形
				RedrawAllShapes(hwnd);
			}
			break;

		case GraphicMode::Scale:
			//实时预览缩放
			if (is_drawing && selectedShape >= 0 && selectedShape < (int)gShapes.size() && hasOriginalShape)
			{
				int dx = x - pos_start.x;
				float scaleFactor = 1.0f + dx * 0.01f;

				if (scaleFactor > 0.1f && scaleFactor < 10.0f)
				{
					POINT center = GetShapeCenter(originalShape);

					Shape tempShape = originalShape;

					// 先平移到原点，缩放，再平移回去
					float M1[3][3], M2[3][3], M3[3][3], temp[3][3], M[3][3];
					GraphicFunc::Transform::GetTranslateMatrix((float)-center.x, (float)-center.y, M1);
					GraphicFunc::Transform::GetScaleMatrix(scaleFactor, scaleFactor, M2);
					GraphicFunc::Transform::GetTranslateMatrix((float)center.x, (float)center.y, M3);

					// temp = M2 * M1
					for (int i = 0; i < 3; i++)
						for (int j = 0; j < 3; j++)
							temp[i][j] = M2[i][0] * M1[0][j] + M2[i][1] * M1[1][j] + M2[i][2] * M1[2][j];

					// M = M3 * temp
					for (int i = 0; i < 3; i++)
						for (int j = 0; j < 3; j++)
							M[i][j] = M3[i][0] * temp[0][j] + M3[i][1] * temp[1][j] + M3[i][2] * temp[2][j];

					GraphicFunc::Transform::TransformShape(tempShape.vertices, M);
					gShapes[selectedShape] = tempShape;

					RedrawAllShapes(hwnd);
				}
			}
			break;

		case GraphicMode::Rotate:
			//实时预览旋转
			if (is_drawing && selectedShape >= 0 && selectedShape < (int)gShapes.size() && hasOriginalShape)
			{
				int dx = x - pos_start.x;
				float angle = dx * 0.02f;

				Shape tempShape = originalShape;
				float M[3][3];
				GraphicFunc::Transform::GetRotateAroundPointMatrix(
					angle,
					(float)pivot_point.x,
					(float)pivot_point.y,
					M
				);

				GraphicFunc::Transform::TransformShape(tempShape.vertices, M);
				gShapes[selectedShape] = tempShape;

				RedrawAllShapes(hwnd);
			}
			break;
		}
		return 0;
	}

	case WM_LBUTTONUP:
	{
		int x = LOWORD(lParam);
		int y = HIWORD(lParam);
		pos_end.x = x;
		pos_end.y = y;
		bool was_drawing = is_drawing;
		is_drawing = false;

		switch (graphic_modes)
		{
		case GraphicMode::LineBresenham:
			GraphicFunc::Line::DrawLineBresenham(pos_start.x, pos_start.y, pos_end.x, pos_end.y, renderer);
			AddShape_line(gShapes, { pos_start.x,pos_start.y }, { pos_end.x,pos_end.y });
			break;

		case GraphicMode::LineMidpoint:
			GraphicFunc::Line::DrawLineMidpoint(pos_start.x, pos_start.y, pos_end.x, pos_end.y, renderer);
			AddShape_line(gShapes, { pos_start.x,pos_start.y }, { pos_end.x,pos_end.y });
			break;

		case GraphicMode::CircleBresenham:
			GraphicFunc::Circle::DrawCircleBresenham(pos_start.x, pos_start.y, pos_end.x, pos_end.y, renderer);
			break;

		case GraphicMode::CircleMidpoint:
			GraphicFunc::Circle::DrawCircleMidpoint(pos_start.x, pos_start.y, pos_end.x, pos_end.y, renderer);
			break;

		case GraphicMode::Rectangle:
			GraphicFunc::Rectang::DrawRectangle(pos_start.x, pos_start.y, pos_end.x, pos_end.y, renderer);
			break;

		case GraphicMode::Brush:
			//尾端防止未触发最后一次鼠标移动
			if (was_drawing && (x != pos_end.x || y != pos_end.y))
			{
				GraphicFunc::Brush::DrawFreeLine(pos_end.x, pos_end.y, x, y, renderer);
			}
			break;

		case GraphicMode::RandomCurve:
			//暂时替代
			//GraphicFunc::DrawArcD2D(pos_start.x, pos_start.y, pos_end.x, pos_end.y, renderer);
			GraphicFunc::Line::DrawLineBresenham(pos_start.x, pos_start.y, pos_end.x, pos_end.y, renderer);
			break;

		case GraphicMode::FillScanLine:
			GraphicFunc::MarkPoint::CleanMarkPoint(control_points, renderer);
			GraphicFunc::Fill::FillRandomShapeScanline(control_points, renderer);
			break;

		case GraphicMode::FillFence:
			GraphicFunc::MarkPoint::CleanMarkPoint(control_points, renderer);
			GraphicFunc::Fill::FillRandomShapeFence(control_points, renderer);
			break;

		case GraphicMode::BSpline:
			GraphicFunc::MarkPoint::CleanMarkPoint(control_points, renderer);
			GraphicFunc::Bspline::DrawRandomBSpline(control_points, renderer);
			break;

		case GraphicMode::RandomPolygon:
			GraphicFunc::MarkPoint::CleanMarkPoint(control_points, renderer);
			GraphicFunc::Polygon::DrawRandomPolygon(control_points, renderer);
			AddShape_polygon(gShapes, control_points);
			control_points.clear();
			break;

		case GraphicMode::Translate:
		case GraphicMode::Scale:
		case GraphicMode::Rotate:
			//变换已在预览中完成，所以抬起按键时只需清理状态并重绘
			if (selectedShape >= 0 && selectedShape < (int)gShapes.size() && hasOriginalShape)
			{
				// 如果没有移动鼠标，恢复原始状态
				if (x == pos_start.x && y == pos_start.y)
				{
					gShapes[selectedShape] = originalShape;
				}
				RedrawAllShapes(hwnd);
			}
			hasOriginalShape = false;
			break;

		case GraphicMode::ClipLine_CohenSutherland:
		{	//————————CS直线裁切
			POINT left_top, right_bottom;
			left_top.x = min(pos_start.x, pos_end.x);
			left_top.y = min(pos_start.y, pos_end.y);
			right_bottom.x = max(pos_start.x, pos_end.x);
			right_bottom.y = max(pos_start.y, pos_end.y);

			for (auto& s : gShapes)
			{
				if (s.type == ShapeType::Line)
				{
					int x0 = s.vertices[0].x;
					int y0 = s.vertices[0].y;
					int x1 = s.vertices[1].x;
					int y1 = s.vertices[1].y;

					if (GraphicFunc::Clip::clip_CohenSutherland(x0, y0, x1, y1, left_top, right_bottom))
					{
						s.vertices[0] = { x0, y0 };
						s.vertices[1] = { x1, y1 };
					}
					else
					{
						s.vertices.clear();
					}
				}
			}
			RedrawAllShapes(hwnd);
			GraphicFunc::Rectang::DrawRectangle(pos_start.x, pos_start.y, pos_end.x, pos_end.y, renderer, Color(0.8f, 0.8f, 0.8f, 0.3f));
			break;
		}

		case GraphicMode::ClipLine_MidpointSubdivision:
		{	//————————中点分割直线裁切
			POINT left_top, right_bottom;
			left_top.x = min(pos_start.x, pos_end.x);
			left_top.y = min(pos_start.y, pos_end.y);
			right_bottom.x = max(pos_start.x, pos_end.x);
			right_bottom.y = max(pos_start.y, pos_end.y);

			for (auto& s : gShapes)
			{
				if (s.type == ShapeType::Line)
				{
					int x0 = s.vertices[0].x;
					int y0 = s.vertices[0].y;
					int x1 = s.vertices[1].x;
					int y1 = s.vertices[1].y;

					if (GraphicFunc::Clip::clip_MidpointSubdivision(x0, y0, x1, y1, left_top, right_bottom))
					{
						s.vertices[0] = { x0, y0 };
						s.vertices[1] = { x1, y1 };
					}
					else
					{
						s.vertices.clear();
					}
				}
			}
			RedrawAllShapes(hwnd);
			GraphicFunc::Rectang::DrawRectangle(pos_start.x, pos_start.y, pos_end.x, pos_end.y, renderer, Color(0.8f, 0.8f, 1.0f, 0.3f));
			break;
		}

		case GraphicMode::ClipPolygon_SutherlandHodgman:
		{	//————————SH多边形裁切
			POINT left_top, right_bottom;
			left_top.x = min(pos_start.x, pos_end.x);
			left_top.y = min(pos_start.y, pos_end.y);
			right_bottom.x = max(pos_start.x, pos_end.x);
			right_bottom.y = max(pos_start.y, pos_end.y);

			std::vector<Shape> newShapes;
			for (auto& s : gShapes)
			{
				if (s.type == ShapeType::Polygon && s.vertices.size() >= 3)
				{
					auto clipped = GraphicFunc::Clip::SutherlandHodgmanClip(s.vertices, left_top, right_bottom);
					if (!clipped.empty())
					{
						Shape sp;
						sp.type = ShapeType::Polygon;
						sp.vertices = clipped;
						newShapes.push_back(sp);
					}
					// 否则剪成空：删除（不加入 newShapes）
				}
				else
				{
					// 非多边形/直线保持不变
					newShapes.push_back(s);
				}
			}
			gShapes.swap(newShapes);
			RedrawAllShapes(hwnd);
			// 绘制裁剪框作为提示
			GraphicFunc::Rectang::DrawRectangle(pos_start.x, pos_start.y, pos_end.x, pos_end.y, renderer, Color(0.8f, 0.8f, 0.8f, 0.3f));
			break;
		}

		case GraphicMode::ClipPolygon_WeilerAtherton:
		{	//————————WA多边形裁切
			POINT left_top, right_bottom;
			left_top.x = min(pos_start.x, pos_end.x);
			left_top.y = min(pos_start.y, pos_end.y);
			right_bottom.x = max(pos_start.x, pos_end.x);
			right_bottom.y = max(pos_start.y, pos_end.y);

			std::vector<Shape> newShapes;
			for (auto& s : gShapes)
			{
				if (s.type == ShapeType::Polygon && s.vertices.size() >= 3)
				{
					auto polys = GraphicFunc::Clip::WeilerAthertonClip(s.vertices, left_top, right_bottom);
					if (!polys.empty())
					{
						// 如果分成多个多边形，就把第一个作为替换，剩余的 push_back 为新的图元
						bool first = true;
						for (auto& poly : polys)
						{
							if (poly.size() < 3) continue;
							Shape sp;
							sp.type = ShapeType::Polygon;
							sp.vertices = poly;
							if (first)
							{
								newShapes.push_back(sp);
								first = false;
							}
							else
							{
								newShapes.push_back(sp);
							}
						}
					}
					// 否则裁剪为空则不保存
				}
				else
				{
					// 非多边形/直线保持不变
					newShapes.push_back(s);
				}
			}
			gShapes.swap(newShapes);
			RedrawAllShapes(hwnd);
			GraphicFunc::Rectang::DrawRectangle(pos_start.x, pos_start.y, pos_end.x, pos_end.y, renderer, Color(0.8f, 0.8f, 0.8f, 0.3f));
			break;
		}

		default:
			break;
		}
		//结束switch (graphic_modes)
		InvalidateRect(hwnd, NULL, FALSE);
		return 0;
	}

	case WM_CREATE:
		renderer.Initialize(hwnd);
		CreateUIControls(hwnd);
		return 0;

	case WM_PAINT:
		renderer.Present();
		ValidateRect(hwnd, NULL);
		return 0;

	case WM_DESTROY:
		renderer.Cleanup();
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR pCmdLine, int nCmdShow)
{
	WNDCLASS wnd_class = {};
	wnd_class.lpfnWndProc = WinProc;
	wnd_class.lpszClassName = L"Experiment3";
	wnd_class.hInstance = hInstance;
	RegisterClass(&wnd_class);

	main_hwnd = CreateWindowExW(
		0,
		L"Experiment3",
		L"202311534黄贵宏",
		WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MAXIMIZEBOX | WS_CLIPCHILDREN,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		1280,
		720,
		nullptr,
		nullptr,
		hInstance,
		nullptr
	);

	ShowWindow(main_hwnd, nCmdShow);
	UpdateWindow(main_hwnd);

	MSG msg = {};
	while (GetMessage(&msg, nullptr, 0, 0) > 0)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}