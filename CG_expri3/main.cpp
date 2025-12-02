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
GraphicMode graphic_modes = GraphicMode::LineBresenham;

HWND main_hwnd = nullptr;

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
				SetWindowTextW(label_OperationMode, L"平移");

			case 12:
				graphic_modes = GraphicMode::Scale;
				SetWindowTextW(label_OperationMode, L"缩放");

			case 13:
				graphic_modes = GraphicMode::Rotate;
				SetWindowTextW(label_OperationMode, L"旋转");

			default:
				graphic_modes = GraphicMode::LineBresenham;
				SetWindowTextW(label_OperationMode, L"按下鼠标左键位置为线段起点，释放鼠标左键位置为线段终点");
				break;
			}
		}
		return 0;
	}

	case WM_LBUTTONDOWN:
		pos_start.x = LOWORD(lParam);
		pos_start.y = HIWORD(lParam);
		is_drawing = true;
		//画笔模式，线段连续，初始化上一点
		if (graphic_modes == GraphicMode::Brush)
		{
			pos_end = pos_start;
		}
		//选中图形
		for (int i = 0; i < gShapes.size(); i++)
		{
			if (PointInsideShape({ LOWORD(lParam),HIWORD(lParam) }, gShapes[i]))
			{
				selectedShape = i;
				break;
			}
		}
		return 0;

	case WM_RBUTTONDOWN:
	{
		int x = LOWORD(lParam);
		int y = HIWORD(lParam);

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
		switch (graphic_modes)
		{
		case GraphicMode::Brush:
			if (is_drawing)
			{
				int x = LOWORD(lParam);
				int y = HIWORD(lParam);
				if (x != pos_end.x || y != pos_end.y)
				{
					GraphicFunc::Brush::DrawFreeLine(pos_end.x, pos_end.y, x, y, renderer);
					pos_end.x = x;
					pos_end.y = y;
				}
			}
			break;

		default:
			break;
		}
		return 0;

	case WM_LBUTTONUP:
	{
		int x = LOWORD(lParam);
		int y = HIWORD(lParam);
		pos_end.x = LOWORD(lParam);
		pos_end.y = HIWORD(lParam);
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
			//GraphicFunc::FillRectangleScanline(pos_start.x, pos_start.y, pos_end.x, pos_end.y, renderer);
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
			break;

		case GraphicMode::Translate:
		{
			float M[3][3];
			int dx = pos_end.x - pos_start.x;
			int dy = pos_end.y - pos_start.y;
			GraphicFunc::Transform::ApplyMatrix({ dx,dy }, M);
			GraphicFunc::Transform::TransformShape(gShapes[selectedShape].vertices, M);
			renderer.Clear();
			for (auto& s : gShapes)
			{
				if (s.type == ShapeType::Line)
				{
					GraphicFunc::Line::DrawLineBresenham(s.vertices[0].x, s.vertices[0].y, s.vertices[1].x, s.vertices[1].y, renderer);
				}
			}
		}
		}
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
		L"简易绘图",
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