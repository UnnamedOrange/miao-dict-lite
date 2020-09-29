#include <Windows.h>
#undef min
#undef max
#include <windowsx.h>
#include "utils/window.hpp"
#include "utils/direct_ui.hpp"
#include "utils/direct_ui_window.hpp"

#include "exit_button.hpp"
#include "icon_button.hpp"
#include "vessel.hpp"

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
		SetLayeredWindowAttributes(hwnd, NULL, 235, LWA_ALPHA);

		RECT wa = work_area();
		right(wa.right - dpi(16));
		left(wa.right - dpi(300));
		bottom(wa.bottom - dpi(16));
		top(wa.bottom - dpi(200));

		vessel_1 = s->build_dep_widget<vessel>();
		s->contents->widgets.push_back(vessel_1);
		vessel_1->resize(s->cx, s->cy);

		auto bg_rect = s->build_dep_widget<rect>();
		vessel_1->widgets.push_back(bg_rect);
		bg_rect->move(0, 0);
		bg_rect->resize(s->cx, s->cy);
		bg_rect->brush_color = color(247u, 228u, 172u);

		exit_button_1 = s->build_dep_widget<exit_button>();
		vessel_1->widgets.push_back(exit_button_1);
		exit_button_1->callback = [this]()
		{
			PostMessageW(this->hwnd, WM_QUIT, NULL, NULL);
		};
		exit_button_1->move(8, 8);

		option_button_1 = s->build_dep_widget<icon_button>();
		vessel_1->widgets.push_back(option_button_1);
		option_button_1->icon = 0xe700;
		option_button_1->move(32, 8);
		option_button_1->set_visible(false);
		vessel_1->hover_to_show.push_back(option_button_1);

		return TRUE;
	}
	void OnDestroy(HWND hwnd)
	{
		PostQuitMessage(0);
	}

private:
	std::shared_ptr<vessel> vessel_1;
	std::shared_ptr<exit_button> exit_button_1;
	std::shared_ptr<icon_button> option_button_1;

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