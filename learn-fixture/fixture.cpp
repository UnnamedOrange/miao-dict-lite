#include <Windows.h>
#undef min
#undef max
#include <windowsx.h>
#include "utils/window.hpp"
#include "utils/direct_ui.hpp"
#include "utils/direct_ui_window.hpp"

using namespace direct_ui;

class fixture_window final : public dui_window
{
	static constexpr int cx = 500;
	virtual INT_PTR DuiWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) override
	{
		switch (message)
		{
			HANDLE_MSG(hwnd, WM_CREATE, OnCreate);
			HANDLE_MSG(hwnd, WM_DESTROY, OnDestroy);
		case WM_DISPLAYCHANGE:
		case WM_DPICHANGED:
		{
			RECT wa = work_area();
			right(wa.right - dpi(16));
			left(wa.right - dpi(300));
			bottom(wa.bottom - dpi(16));
			top(wa.bottom - dpi(200));
			break;
		}

		default:
			return DefWindowProcW(hwnd, message, wParam, lParam);
		}
		return 0;
	}
	BOOL OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct)
	{
		caption(L"fixture");
		SetWindowLongW(hwnd, GWL_STYLE,
			WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_POPUP | WS_SYSMENU);
		SetForegroundWindow(hwnd);
		SetWindowPos(hwnd, HWND_TOPMOST, NULL, NULL, NULL, NULL, SWP_NOMOVE | SWP_NOSIZE | SWP_NOREDRAW);
		SetWindowLongW(hwnd, GWL_EXSTYLE, WS_EX_LAYERED | WS_EX_TOPMOST);
		SetLayeredWindowAttributes(hwnd, NULL, 254, LWA_ALPHA);

		RECT wa = work_area();
		right(wa.right - dpi(16));
		left(wa.right - dpi(300));
		bottom(wa.bottom - dpi(16));
		top(wa.bottom - dpi(200));

		auto bg_rect = s->build_dep_widget<rect>();
		s->contents->widgets.push_back(bg_rect);
		bg_rect->move(0, 0);
		bg_rect->resize(s->cx, s->cy);
		bg_rect->brush_color = 0xffffffff;

		return TRUE;
	}
	void OnDestroy(HWND hwnd)
	{
		PostQuitMessage(0);
	}

private:
	std::shared_ptr<button> b1;

public:
	fixture_window()
	{

	}
};

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE, _In_ PWSTR, _In_ int)
{
	SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
	fixture_window main_window;
	main_window.create();
	return main_window.message_loop();
}