#pragma once

#include "window.hpp"
#include "direct_ui.hpp"

namespace direct_ui
{
	class dui_window : public window
	{
	private:
		inline static scene_factory dui_scene_factory;
		std::shared_ptr<scene> _builtin_scene;
	public:
		const std::shared_ptr<scene>& builtin_scene{ _builtin_scene };
		const std::shared_ptr<scene>& s{ _builtin_scene };

	private:
		int capture_count{};

	private:
		virtual INT_PTR WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) override final
		{
			switch (message)
			{
			case WM_CREATE:
			{
				_builtin_scene = dui_scene_factory.build_hwnd_scene(hwnd);
				builtin_scene->resize(cwidth(), cheight());
				break;
			}
			case WM_DESTROY:
			{
				_builtin_scene.reset();
				break;
			}
			case WM_PAINT:
			{
				builtin_scene->on_paint();
				ValidateRect(hwnd, nullptr);
				return 0;
			}
			case WM_SIZE:
			{
				HANDLE_WM_SIZE(hwnd, wParam, lParam,
					[this](HWND hwnd, UINT state, int cx, int cy)
					{
						builtin_scene->resize(cx, cy);
					});
				break;
			}
			case WM_MOUSEMOVE:
			{
				HANDLE_WM_MOUSEMOVE(hwnd, wParam, lParam,
					[this](HWND hwnd, int x, int y, UINT keyFlags)
					{
						TRACKMOUSEEVENT tme{ sizeof(tme) };
						tme.hwndTrack = hwnd;
						tme.dwFlags = TME_LEAVE;
						tme.dwHoverTime = 0;
						TrackMouseEvent(&tme);
						builtin_scene->on_mouse_move(x, y);
					});
				break;
			}
			case WM_MOUSELEAVE:
			{
				builtin_scene->on_mouse_leave();
				break;
			}
			case WM_LBUTTONDOWN:
			{
				HANDLE_WM_LBUTTONDOWN(hwnd, wParam, lParam,
					[this](HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags)
					{
						if (capture_count++)
							SetCapture(hwnd);
						builtin_scene->on_left_down(x, y);
					});
				break;
			}
			case WM_LBUTTONUP:
			{
				HANDLE_WM_LBUTTONUP(hwnd, wParam, lParam,
					[this](HWND hwnd, int x, int y, UINT keyFlags)
					{
						builtin_scene->on_left_up(x, y);
						if (--capture_count)
							ReleaseCapture();
					});
				break;
			}
			}
			return DuiWindowProc(hwnd, message, wParam, lParam);
		}
		virtual INT_PTR DuiWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) = 0;
	};
}