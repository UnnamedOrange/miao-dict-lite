#pragma once

#include <vector>
#include <memory>
#include <unordered_set>
#include <type_traits>
#include <functional>
#include <chrono>
#include <optional>
#include <concepts>
#include <ranges>

#if _MSVC_LANG
#include <Windows.h>
#undef min
#undef max
#include <d2d1.h>
#include <d2d1_1.h>
#pragma comment(lib, "d2d1.lib")
#endif

template <typename T>
class reversed : public std::bidirectional_iterator_tag
{
private:
	T& iterable;
public:
	reversed(T& iterable) : iterable(iterable) {}
	reversed(const reversed&) = delete;
	reversed(reversed&&) = delete;
	reversed<T>& operator=(const reversed&) = delete;
	reversed<T>& operator=(reversed&&) = delete;
	auto begin() const { return iterable.rbegin(); }
	auto end() const { return iterable.rend(); }
	auto cbegin() const { return iterable.crbegin(); }
	auto cend() const { return iterable.crend(); }
	auto rbegin() const { return iterable.begin(); }
	auto rend() const { return iterable.end(); }
	auto crbegin() const { return iterable.cbegin(); }
	auto crend() const { return iterable.cend(); }
};

#if _MSVC_LANG
class timer
{
	inline static std::unordered_set<UINT_PTR> exists;
	static void CALLBACK TimerProc(HWND hwnd, UINT message, UINT_PTR id, DWORD elapse)
	{
		if (!exists.count(id))
			return;
		reinterpret_cast<timer*>(id)->callback();
	}
public:
	timer(std::function<void()> callback = [] {}) : callback(callback)
	{
		exists.insert(reinterpret_cast<UINT_PTR>(this));
	}
	~timer()
	{
		kill();
		exists.erase(reinterpret_cast<UINT_PTR>(this));
	}
	timer(const timer&) = delete;
	timer(timer&&) = delete;
	timer& operator=(const timer&) = delete;
	timer& operator=(timer&&) = delete;
public:
	std::function<void()> callback;
	void set(DWORD elapse) const
	{
		SetTimer(nullptr, reinterpret_cast<UINT_PTR>(this), elapse, &timer::TimerProc);
	}
	void kill() const
	{
		KillTimer(nullptr, reinterpret_cast<UINT_PTR>(this));
	}
};
#endif

namespace direct_ui
{
	using real = float;

	class logic_widget
	{
	public:
		virtual ~logic_widget() {}
	private:
		real _x{};
		real _y{};
		real _cx{};
		real _cy{};
	public:
		const real& x{ _x };
		const real& y{ _y };
		const real& cx{ _cx };
		const real& cy{ _cy };
	public:
		void move(std::optional<real> x, std::optional<real> y)
		{
			if (x)
				_x = *x;
			if (y)
				_y = *y;
		}
		void resize(std::optional<real> cx, std::optional<real> cy)
		{
			on_resize(cx ? *cx : _cx, cy ? *cy : _cy);
			if (cx)
				_cx = *cx;
			if (cy)
				_cy = *cy;
		}
	private:
		bool _is_focused{};
		bool _is_activated{};
	public:
		bool is_focusable{ true };
	public:
		const bool& is_foucsed{ _is_focused };
		const bool& is_activated{ _is_activated };
	public:
		bool set_focus()
		{
			if (is_focusable)
			{
				_is_focused = true;
				return true;
			}
			return false;
		}
		void kill_focus()
		{
			_is_focused = false;
		}
		void activate()
		{
			_is_activated = true;
		}
		void deactivate()
		{
			_is_activated = false;
		}

	public:
		virtual void on_resize(real cx, real cy) {}
		virtual void on_left_down(real x, real y) {}
		virtual void on_left_up(real x, real y) {}
		virtual void on_mid_down(real x, real y) {}
		virtual void on_mid_up(real x, real y) {}
		virtual void on_right_down(real x, real y) {}
		virtual void on_right_up(real x, real y) {}
		virtual void on_mouse_move(real x, real y) {}
		virtual void on_mouse_hover() {}
		virtual void on_mouse_leave() {}
		virtual void on_key_down(int vk) {}
		virtual void on_key_up(int vk) {}
		virtual void on_update(std::chrono::high_resolution_clock::duration interval) {}
		virtual bool on_hittest(real x, real y) { return true; }

		friend class scene;
	private:
		std::weak_ptr<scene> _ancestor;
	public:
		const std::weak_ptr<scene>& ancestor{ _ancestor };
		void require_update();
	};
	template <typename logic_t>
	class dep_widget_interface
	{
	private:
		virtual void init_resources() {}
	public:
		virtual ~dep_widget_interface() {}

	public:
		virtual void on_paint() const = 0;

		friend class scene;
	};
	template <typename logic_t>
	class dep_widget : public dep_widget_interface<logic_t>
	{
	protected:
#if _MSVC_LANG
		ID2D1Factory* pFactory{};
		ID2D1RenderTarget* pRenderTarget{};
#endif
		friend class scene;
	};
	using dep_widget_base = dep_widget<logic_widget>;
	template <typename dep_t>
	inline std::shared_ptr<logic_widget> to_logic(std::shared_ptr<dep_t> dep)
		requires std::is_base_of_v<dep_widget_base, dep_t> || std::is_same_v<dep_widget_base, dep_t>
	{
		return std::dynamic_pointer_cast<logic_widget>(dep);
	}

#if _MSVC_LANG
	class scene
	{
		ID2D1Factory* pFactory;
		ID2D1RenderTarget* pRenderTarget;
		std::weak_ptr<scene> self;

	public:
		std::vector<std::shared_ptr<dep_widget_base>> widgets;

	public:
		scene(ID2D1Factory* pFactory, ID2D1RenderTarget* pRenderTarget) :
			pFactory(pFactory),
			pRenderTarget(pRenderTarget)
		{

		}
		~scene()
		{
			if (pRenderTarget)
				pRenderTarget->Release();
		}

	private:
		timer update_timer{ [this]()
		{
			if (update_flag)
				on_update();
			update_flag = false;
			update_timer.kill();
		} };
		bool update_flag{};
		std::chrono::high_resolution_clock::time_point pre{};
	public:
		void update()
		{
			if (!update_flag)
			{
				pre = std::chrono::high_resolution_clock::now();
				update_flag = true;
			}
			update_timer.set(0);
		}
		void resize(int width, int height)
		{
			reinterpret_cast<ID2D1HwndRenderTarget*>(pRenderTarget)->Resize(D2D1::SizeU(width, height));
		}
	public:
		auto on_hittest(int x, int y) const
		{
			std::shared_ptr<dep_widget_base> ret;
			for (const auto& widget : reversed(widgets))
			{
				auto logic = to_logic(widget);
				if (logic->x <= x && x < logic->x + logic->cx &&
					logic->y <= y && y < logic->y + logic->cy &&
					logic->on_hittest(x - logic->x, y - logic->y))
				{
					ret = widget;
					break;
				}
			}
			return ret;
		}

	private:
		std::shared_ptr<dep_widget_base> mouse_on;
		std::pair<std::shared_ptr<dep_widget_base>, int> mouse_capture{};
		std::shared_ptr<dep_widget_base> focused;

	public:
		void on_paint()
		{
			pRenderTarget->BeginDraw();
			for (const auto& widget : widgets)
				widget->on_paint();
			pRenderTarget->EndDraw();
		}
		void on_mouse_move(int x, int y)
		{
			auto on_which = on_hittest(x, y);
			if (mouse_capture.second)
				if (mouse_capture.first != on_which)
					on_which.reset();
			if (on_which)
			{
				if (on_which != mouse_on)
				{
					if (mouse_on)
					{
						auto logic = to_logic(mouse_on);
						logic->on_mouse_leave();
					}
					mouse_on = on_which;
					auto logic = to_logic(mouse_on);
					logic->on_mouse_hover();
				}
				auto logic = to_logic(on_which);
				logic->on_mouse_move(x - logic->x, y - logic->y);
			}
			else if (mouse_on)
			{
				auto logic = to_logic(mouse_on);
				logic->on_mouse_leave();
				mouse_on.reset();
			}
		}
		void on_mouse_leave()
		{
			mouse_on.reset();
		}
		void on_left_down(int x, int y)
		{
			auto on_which = on_hittest(x, y);
			if (mouse_capture.second)
				on_which = mouse_capture.first;
			if (on_which)
			{
				auto logic = to_logic(on_which);
				bool is_focus = logic->set_focus();
				if (is_focus && focused && focused != on_which)
				{
					auto logic = to_logic(focused);
					logic->kill_focus();
					focused.reset();
				}
				if (is_focus)
					focused = on_which;
				logic->on_left_down(x - logic->x, y - logic->y);
				mouse_capture.first = on_which;
				mouse_capture.second++;
			}
		}
		void on_left_up(int x, int y)
		{
			auto logic = to_logic(mouse_capture.first);
			logic->on_left_up(x - logic->x, y - logic->y);
			if (!(--mouse_capture.second))
				mouse_capture.first.reset();
		}
		void on_mid_down(int x, int y)
		{
			auto on_which = on_hittest(x, y);
			if (mouse_capture.second)
				on_which = mouse_capture.first;
			if (on_which)
			{
				auto logic = to_logic(on_which);
				bool is_focus = logic->set_focus();
				if (is_focus && focused && focused != on_which)
				{
					auto logic = to_logic(focused);
					logic->kill_focus();
					focused.reset();
				}
				if (is_focus)
					focused = on_which;
				logic->on_mid_down(x - logic->x, y - logic->y);
				mouse_capture.first = on_which;
				mouse_capture.second++;
			}
		}
		void on_mid_up(int x, int y)
		{
			auto logic = to_logic(mouse_capture.first);
			logic->on_mid_up(x - logic->x, y - logic->y);
			if (!(--mouse_capture.second))
				mouse_capture.first.reset();
		}
		void on_right_down(int x, int y)
		{
			auto on_which = on_hittest(x, y);
			if (mouse_capture.second)
				on_which = mouse_capture.first;
			if (on_which)
			{
				auto logic = to_logic(on_which);
				bool is_focus = logic->set_focus();
				if (is_focus && focused && focused != on_which)
				{
					auto logic = to_logic(focused);
					logic->kill_focus();
					focused.reset();
				}
				if (is_focus)
					focused = on_which;
				logic->on_right_down(x - logic->x, y - logic->y);
				mouse_capture.first = on_which;
				mouse_capture.second++;
			}
		}
		void on_right_up(int x, int y)
		{
			auto logic = to_logic(mouse_capture.first);
			logic->on_right_up(x - logic->x, y - logic->y);
			if (!(--mouse_capture.second))
				mouse_capture.first.reset();
		}
		void on_set_focus()
		{
			for (const auto& widget : widgets)
				to_logic(widget)->activate();
		}
		void on_kill_focus()
		{
			for (const auto& widget : widgets)
				to_logic(widget)->deactivate();
		}
		void on_update()
		{
			auto now = std::chrono::high_resolution_clock::now();
			auto period = now - pre;
			for (const auto& widget : widgets)
				to_logic(widget)->on_update(period);
		}
	public:
		template <typename dep_widget_t>
		std::shared_ptr<dep_widget_t> build_dep_widget()
			requires std::is_base_of_v<dep_widget_base, dep_widget_t>
		{
			using decayed = std::decay_t<dep_widget_t>;
			auto ret = std::make_shared<decayed>();
			ret->_ancestor = self;
			ret->pFactory = pFactory;
			ret->pRenderTarget = pRenderTarget;
			ret->init_resources();
			return ret;
		}

		friend class scene_factory;
	};
	class scene_factory
	{
		ID2D1Factory* pFactory;

	public:
		scene_factory()
		{
			if (FAILED(D2D1CreateFactory(
				D2D1_FACTORY_TYPE::D2D1_FACTORY_TYPE_MULTI_THREADED, &pFactory)))
				throw std::runtime_error("Fail to D2D1CreateFactory.");
		}
		scene_factory(const scene_factory&) = delete;
		scene_factory(scene_factory&&) = delete;
		scene_factory& operator=(const scene_factory&) = delete;
		scene_factory& operator=(scene_factory&&) = delete;
		~scene_factory()
		{
			if (pFactory)
				pFactory->Release();
		}

	public:
		std::shared_ptr<scene> build_hwnd_scene(HWND hwnd)
		{
			ID2D1HwndRenderTarget* pRenderTarget;
			if (FAILED(pFactory->CreateHwndRenderTarget(D2D1::RenderTargetProperties(),
				D2D1::HwndRenderTargetProperties(hwnd, D2D1::SizeU()),
				&pRenderTarget)))
				throw std::runtime_error("Fail to CreateHwndRenderTarget.");
			pRenderTarget->SetDpi(USER_DEFAULT_SCREEN_DPI, USER_DEFAULT_SCREEN_DPI);
			auto ret = std::make_shared<scene>(pFactory, pRenderTarget);
			ret->self = ret;
			return ret;
		}
	};
#endif
	inline void direct_ui::logic_widget::require_update()
	{
		ancestor.lock()->update();
	}

	class logic_rect : public logic_widget
	{
	public:
		unsigned int brush_color{};
		unsigned int pen_color{};
		real pen_size{};
		logic_rect()
		{
			is_focusable = false;
		}
	};
	class logic_button : public logic_widget
	{
	protected:
		bool is_mouse_hover{};
		int is_mouse_down{};
	public:
		std::function<void()> callback{ [] {} };
	public:
		virtual void on_mouse_hover() override
		{
			is_mouse_hover = true;
		}
		virtual void on_mouse_leave() override
		{
			is_mouse_hover = false;
		}
		virtual void on_left_down(real x, real y) override
		{
			is_mouse_down++;
		}
		virtual void on_left_up(real x, real y) override
		{
			is_mouse_down--;
			if (is_mouse_hover)
				callback();
		}
	};

#if _MSVC_LANG
	template <>
	class dep_widget<logic_button> : public logic_button, public dep_widget_base
	{
		ID2D1SolidColorBrush* brush{};

	public:
		virtual void on_paint() const override
		{
			pRenderTarget->FillRectangle(D2D1::RectF(x, y, x + cx, y + cy), brush);
		}

	private:
		virtual void init_resources() override
		{
			pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::AliceBlue), &brush);
		}
	public:
		~dep_widget()
		{
			if (brush)
				brush->Release();
		}

		friend class scene;
	};
	using button = dep_widget<logic_button>;
	template <>
	class dep_widget<logic_rect> : public logic_rect, public dep_widget_base
	{
	public:
		virtual void on_paint() const override
		{
			auto rect = D2D1::Rect(x, y, x + cx, y + cy);
			{
				ID2D1SolidColorBrush* brush;
				pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(brush_color, (brush_color >> 24) / 255.f), &brush);
				pRenderTarget->FillRectangle(rect, brush);
				brush->Release();
			}
			if (pen_size)
			{
				ID2D1SolidColorBrush* brush;
				pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(pen_color, (pen_color >> 24) / 255.f), &brush);
				pRenderTarget->DrawRectangle(rect, brush, pen_size);
				brush->Release();
			}
		}
	};
	using rect = dep_widget<logic_rect>;
#endif

	template <typename logic_t>
	concept has_implimented_dep_widget = std::is_base_of_v<logic_t, dep_widget<logic_t>> &&
		std::is_base_of_v<dep_widget_base, dep_widget<logic_t>>;
	static_assert(has_implimented_dep_widget<logic_rect>);
	static_assert(has_implimented_dep_widget<logic_button>);
}