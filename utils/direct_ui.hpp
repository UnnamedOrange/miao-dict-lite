#pragma once

#include <vector>
#include <memory>
#include <queue>
#include <unordered_set>
#include <unordered_map>
#include <type_traits>
#include <functional>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <optional>
#include <concepts>
#include <ranges>

#include "lock_view.hpp"
#include "code_conv.hpp"

#if _MSVC_LANG
#include <Windows.h>
#undef min
#undef max
#include <d2d1.h>
#include <d2d1_1.h>
#include <dwrite.h>
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")
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

class timer
{
	bool exit{};
	std::atomic<bool> prompt{};
	std::atomic<bool> disabled_once{};
	std::condition_variable cv;
	std::chrono::high_resolution_clock::duration elapse{};
	std::function<void()> callback;

	void thread_routine()
	{
		while (!exit)
		{
			std::mutex mtx;
			std::unique_lock<std::mutex> lck(mtx);
			cv.wait(lck, [&]()->bool
				{
					return elapse.count() || prompt || exit;
				});
			cv.wait_for(lck, elapse, [&]()->bool
				{
					return prompt || exit;
				});
			prompt = false;
			if (exit)
				break;

			if (elapse.count())
			{
				if (disabled_once)
					disabled_once = false;
				else
					callback();
			}
		}
	}
	std::thread timer_thread{ &timer::thread_routine, this };
public:
	timer(std::function<void()> callback = [] {}) : callback(callback)
	{
	}
	~timer()
	{
		exit = true;
		cv.notify_one();
		timer_thread.join();
	}
	timer(const timer&) = delete;
	timer(timer&&) = delete;
	timer& operator=(const timer&) = delete;
	timer& operator=(timer&&) = delete;
public:
	void set(std::chrono::high_resolution_clock::duration elapse, bool right_now = true)
	{
		if (!elapse.count())
			elapse += std::chrono::high_resolution_clock::duration(1);
		this->elapse = elapse;
		prompt = true;
		disabled_once = !right_now;
		cv.notify_one();
	}
	void kill()
	{
		elapse = elapse.zero();
		prompt = true;
		cv.notify_one();
	}
};

namespace direct_ui
{
	using real = float;
	class color
	{
	public:
		real r{};
		real g{};
		real b{};
		real a{ 1 };
	public:
		static constexpr unsigned red_shift = 16;
		static constexpr unsigned green_shift = 8;
		static constexpr unsigned blue_shift = 0;
		static constexpr unsigned alpha_shift = 24;
		static constexpr unsigned red_mask = 0xff << red_shift;
		static constexpr unsigned green_mask = 0xff << green_shift;
		static constexpr unsigned blue_mask = 0xff << blue_shift;
		static constexpr unsigned alpha_mask = 0xff << alpha_shift;
	public:
		constexpr color() = default;
		constexpr color(unsigned int rgb_or_argb, unsigned char a = 0) :
			r{ static_cast<real>((rgb_or_argb & red_mask) >> red_shift) / 255.f },
			g{ static_cast<real>((rgb_or_argb & green_mask) >> green_shift) / 255.f },
			b{ static_cast<real>((rgb_or_argb & blue_mask) >> blue_shift) / 255.f }
		{
			if (a)
				this->a = static_cast<real>(a / 255.f);
			else
				this->a = static_cast<real>((rgb_or_argb & alpha_mask) >> alpha_shift) / 255.f;
		}
		constexpr color(real r, real g, real b, real a = 1.f) :
			r(r), g(g), b(b), a(a) {}
		constexpr color(unsigned r, unsigned g, unsigned b, unsigned a = 255) :
			r(static_cast<real>(r) / 255.f),
			g(static_cast<real>(g) / 255.f),
			b(static_cast<real>(b) / 255.f),
			a(static_cast<real>(a) / 255.f) {}
	public:
		static constexpr color linear_interpolation(const color& c0, const color& c1, real ratio)
		{
			color ret;
			ret.r = c0.r * (1 - ratio) + c1.r * ratio;
			ret.g = c0.g * (1 - ratio) + c1.g * ratio;
			ret.b = c0.b * (1 - ratio) + c1.b * ratio;
			ret.a = c0.a * (1 - ratio) + c1.a * ratio;
			return ret;
		}

#if _MSVC_LANG
	public:
		operator D2D1::ColorF() const
		{
			return D2D1::ColorF(r, g, b, a);
		}
#endif
	};

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
			on_activate();
		}
		void deactivate()
		{
			_is_activated = false;
			on_deactivate();
		}

	public:
		virtual void on_resize(real cx, real cy) {}
		virtual bool on_hittest(real x, real y) { return true; }
		virtual void on_left_down(real x, real y) {}
		virtual void on_left_up(real x, real y) {}
		virtual void on_mid_down(real x, real y) {}
		virtual void on_mid_up(real x, real y) {}
		virtual void on_right_down(real x, real y) {}
		virtual void on_right_up(real x, real y) {}
		virtual void on_mouse_move(real x, real y) {}
		virtual void on_mouse_hover() {}
		virtual void on_mouse_leave() {}
		virtual void on_update(std::chrono::high_resolution_clock::duration elapsed) {}
		virtual void on_activate() {}
		virtual void on_deactivate() {}

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
	class dep_widget : virtual public dep_widget_interface<logic_t>
	{
	protected:
#if _MSVC_LANG
		ID2D1Factory* pFactory{};
		ID2D1RenderTarget* pRenderTarget{};
#endif
		friend class scene;
	};
	using dep_widget_base = dep_widget<logic_widget>;

	template <typename logic_t>
	concept has_implimented_dep_widget = std::is_base_of_v<logic_t, dep_widget<logic_t>> &&
		std::is_base_of_v<dep_widget_base, dep_widget<logic_t>>;

	template <typename logic_t = logic_widget, typename dep_t>
	inline std::shared_ptr<logic_t> to_logic(std::shared_ptr<dep_t> dep)
		requires std::is_base_of_v<dep_widget_base, dep_t> || std::is_same_v<dep_widget_base, dep_t>
	{
		return std::dynamic_pointer_cast<logic_t>(dep);
	}

	class logic_group : virtual public logic_widget
	{
	public:
		std::vector<std::shared_ptr<dep_widget_base>> widgets;

	public:
		auto hittest(real x, real y) const
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

	public:
		std::function<void(real, real)> resize_callback{ [](real, real) {} };
	private:
		std::shared_ptr<dep_widget_base> mouse_on;
		std::pair<std::shared_ptr<dep_widget_base>, int> mouse_capture{};
		std::shared_ptr<dep_widget_base> focused;
	public:
		virtual void on_resize(real cx, real cy) override
		{
			resize_callback(cx, cy);
		}
		virtual void on_left_down(real x, real y) override
		{
			auto on_which = hittest(x, y);
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
		virtual void on_left_up(real x, real y) override
		{
			auto logic = to_logic(mouse_capture.first);
			logic->on_left_up(x - logic->x, y - logic->y);
			if (!(--mouse_capture.second))
				mouse_capture.first.reset();
		}
		virtual void on_mid_down(real x, real y) override
		{
			auto on_which = hittest(x, y);
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
		virtual void on_mid_up(real x, real y) override
		{
			auto logic = to_logic(mouse_capture.first);
			logic->on_mid_up(x - logic->x, y - logic->y);
			if (!(--mouse_capture.second))
				mouse_capture.first.reset();
		}
		virtual void on_right_down(real x, real y) override
		{
			auto on_which = hittest(x, y);
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
		virtual void on_right_up(real x, real y) override
		{
			auto logic = to_logic(mouse_capture.first);
			logic->on_right_up(x - logic->x, y - logic->y);
			if (!(--mouse_capture.second))
				mouse_capture.first.reset();
		}
		virtual void on_mouse_move(real x, real y) override
		{
			auto on_which = hittest(x, y);
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
		virtual void on_mouse_leave() override
		{
			if (mouse_on)
			{
				auto logic = to_logic(mouse_on);
				logic->on_mouse_leave();
				mouse_on.reset();
			}
		}
		virtual void on_update(std::chrono::high_resolution_clock::duration elapsed) override
		{
			for (const auto& widget : widgets)
				to_logic(widget)->on_update(elapsed);
		}
		virtual void on_activate() override
		{
			for (const auto& widget : widgets)
				to_logic(widget)->activate();
		}
		virtual void on_deactivate() override
		{
			for (const auto& widget : widgets)
				to_logic(widget)->deactivate();
		}
	};
#if _MSVC_LANG
	template <>
	class dep_widget<logic_group> : virtual public logic_group, virtual public dep_widget_base
	{
	public:
		virtual void on_paint() const override
		{
			pRenderTarget->PushAxisAlignedClip(D2D1::RectF(0, 0, cx, cy), D2D1_ANTIALIAS_MODE_ALIASED);
			D2D1_MATRIX_3X2_F transform;
			pRenderTarget->GetTransform(&transform);
			for (const auto& widget : widgets)
			{
				auto logic = to_logic(widget);
				auto move = D2D1::Matrix3x2F::Translation(logic->x, logic->y);
				pRenderTarget->SetTransform(transform * move);
				widget->on_paint();
			}
			pRenderTarget->SetTransform(transform);
			pRenderTarget->PopAxisAlignedClip();
		}
	};
	using group = dep_widget<logic_group>;
	static_assert(has_implimented_dep_widget<logic_group>);
#endif

#if _MSVC_LANG
	class scene
	{
		std::weak_ptr<scene> self;
	public:
		ID2D1Factory* pFactory;
		IDWriteFactory* pDWriteFactory;
		ID2D1RenderTarget* pRenderTarget;
		HWND hwnd;

	public:
		std::shared_ptr<group> contents;

	public:
		scene(ID2D1Factory* pFactory, IDWriteFactory* pDWriteFactory, ID2D1RenderTarget* pRenderTarget, HWND hwnd) :
			pFactory(pFactory),
			pDWriteFactory(pDWriteFactory),
			pRenderTarget(pRenderTarget),
			hwnd(hwnd),
			contents(build_dep_widget<group>())
		{

		}
		~scene()
		{
			if (pRenderTarget)
				pRenderTarget->Release();
		}

	private:
		void timer_routine()
		{
			auto new_pre = std::chrono::high_resolution_clock::now();
			bool into = update_flag;
			update_flag = false;
			if (into)
				on_update();
			if (!update_flag)
				update_timer.kill();
			else
				pre = new_pre;
		}
		timer update_timer{ std::bind(&scene::timer_routine, this) };
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
			update_timer.set(std::chrono::milliseconds(10));
		}
	private:
		real _cx{}, _cy{}, _scale{};
	public:
		const real& cx{ _cx };
		const real& cy{ _cy };
		const real& scale{ _scale };
	public:
		void resize(int width, int height, int dpi)
		{
			_scale = static_cast<real>(dpi) / USER_DEFAULT_SCREEN_DPI;
			_cx = width / scale;
			_cy = height / scale;
			reinterpret_cast<ID2D1HwndRenderTarget*>(pRenderTarget)->Resize(D2D1::SizeU(width, height));
			pRenderTarget->SetDpi(dpi, dpi);
			to_logic(contents)->resize(cx, cy);
		}

	public:
		void on_paint()
		{
			pRenderTarget->BeginDraw();
			contents->on_paint();
			pRenderTarget->EndDraw();
		}
		void on_mouse_move(int x, int y)
		{
			contents->on_mouse_move(x / scale, y / scale);
		}
		void on_mouse_leave()
		{
			contents->on_mouse_leave();
		}
		void on_left_down(int x, int y)
		{
			contents->on_left_down(x / scale, y / scale);
		}
		void on_left_up(int x, int y)
		{
			contents->on_left_up(x / scale, y / scale);
		}
		void on_mid_down(int x, int y)
		{
			contents->on_mid_down(x / scale, y / scale);
		}
		void on_mid_up(int x, int y)
		{
			contents->on_mid_up(x / scale, y / scale);
		}
		void on_right_down(int x, int y)
		{
			contents->on_right_down(x / scale, y / scale);
		}
		void on_right_up(int x, int y)
		{
			contents->on_right_up(x / scale, y / scale);
		}
		void on_update()
		{
			auto now = std::chrono::high_resolution_clock::now();
			auto elapsed = now - pre;
			contents->on_update(elapsed);
			if (hwnd)
				InvalidateRect(hwnd, nullptr, FALSE);
		}
		void on_set_focus()
		{
			contents->activate();
		}
		void on_kill_focus()
		{
			contents->deactivate();
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
		IDWriteFactory* pDWriteFactory;

	public:
		scene_factory()
		{
			if (FAILED(D2D1CreateFactory(
				D2D1_FACTORY_TYPE::D2D1_FACTORY_TYPE_MULTI_THREADED, &pFactory)))
				throw std::runtime_error("Fail to D2D1CreateFactory.");
			if (FAILED(DWriteCreateFactory(
				DWRITE_FACTORY_TYPE_SHARED,
				__uuidof(IDWriteFactory),
				reinterpret_cast<IUnknown**>(&pDWriteFactory))))
				throw std::runtime_error("Fail to DWriteCreateFactory.");
		}
		scene_factory(const scene_factory&) = delete;
		scene_factory(scene_factory&&) = delete;
		scene_factory& operator=(const scene_factory&) = delete;
		scene_factory& operator=(scene_factory&&) = delete;
		~scene_factory()
		{
			if (pFactory)
				pFactory->Release();
			if (pDWriteFactory)
				pDWriteFactory->Release();
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
			auto ret = std::make_shared<scene>(pFactory, pDWriteFactory, pRenderTarget, hwnd);
			ret->self = ret;
			return ret;
		}
	};
	inline void direct_ui::logic_widget::require_update()
	{
		if (!ancestor.expired())
			ancestor.lock()->update();
	}
#endif

	class logic_button : virtual public logic_widget
	{
	protected:
		bool is_mouse_hover{};
		int is_mouse_down{};
	public:
		std::function<void()> callback{ [] {} };
		std::u8string caption;
	protected:
		mutable double frame{};
		static constexpr real max_radius = 233;
		mutable lockfree<std::deque<std::tuple<real, real, real>>> circles;
	public:
		virtual void on_update(std::chrono::high_resolution_clock::duration interval) override
		{
			using namespace std::chrono;
			auto sec = duration_cast<duration<double>>(interval).count();

			{
				double target_frame = 0;
				if (is_mouse_hover && !is_mouse_down)
					target_frame = 100;
				constexpr double speed = 1000;
				double step = speed * sec;
				frame += (target_frame > frame ? step : -step);
				if (!(0 <= frame))
					frame = 0;
				if (!(frame <= 100))
					frame = 100;
				if (std::abs(frame - target_frame) > 1e-6)
					require_update();
			}
			{
				auto view = circles.view();
				constexpr real speed = 400;
				real step = speed * sec;
				for (auto& t : *view)
					std::get<2>(t) += step;
				while (!view->empty())
				{
					if (std::get<2>(view->front()) > max_radius)
						view->pop_front();
					else
						break;
				}
				if (!view->empty())
					require_update();
			}
		}
		virtual void on_mouse_hover() override
		{
			is_mouse_hover = true;
			require_update();
		}
		virtual void on_mouse_leave() override
		{
			is_mouse_hover = false;
			require_update();
		}
		virtual void on_left_down(real x, real y) override
		{
			is_mouse_down++;
			{
				auto view = circles.view();
				view->push_back({ x, y, 0 });
			}
			require_update();
		}
		virtual void on_left_up(real x, real y) override
		{
			is_mouse_down--;
			require_update();
			if (is_mouse_hover)
				callback();
		}
	};
#if _MSVC_LANG
	template <>
	class dep_widget<logic_button> : virtual public logic_button, virtual public dep_widget_base
	{
		ID2D1SolidColorBrush* brush{};
		ID2D1SolidColorBrush* brush_frame{};
		ID2D1SolidColorBrush* brush_font{};
		IDWriteTextFormat* text_format{};

	public:
		virtual void on_paint() const override
		{
			pRenderTarget->FillRectangle(D2D1::RectF(0, 0, cx, cy), brush);
			{
				pRenderTarget->PushAxisAlignedClip(D2D1::RectF(0, 0, cx, cy),
					D2D1_ANTIALIAS_MODE::D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
				auto view = circles.view();
				for (const auto& c : *view)
				{
					const auto& [x, y, r] = c;

					ID2D1SolidColorBrush* circle_brush{};
					real value = 0x7A + (0xCC - 0x7A) * (r / max_radius);
					value /= 255;
					auto color = D2D1::ColorF(value, value, value, 0.5);
					pRenderTarget->CreateSolidColorBrush(color, &circle_brush);
					pRenderTarget->FillEllipse(D2D1::Ellipse(D2D1::Point2F(x, y), r, r), circle_brush);
					circle_brush->Release();
				}
				pRenderTarget->PopAxisAlignedClip();
			}
			if (frame)
			{
				auto properties = D2D1::StrokeStyleProperties();
				pRenderTarget->DrawRectangle(D2D1::RectF(0, 0, cx, cy), brush_frame, 2.0f * frame / 100);
			}
			{
				auto str = code_conv<char8_t, wchar_t>::convert(caption);
				pRenderTarget->DrawTextW(str.c_str(),
					str.length(), text_format,
					D2D1::RectF(0, 0, cx, cy),
					brush_font);
			}
		}

	private:
		virtual void init_resources() override
		{
			pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0xCCCCCC), &brush);
			pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0x7A7A7A), &brush_frame);
			pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0x000000), &brush_font);
			IDWriteFactory* dwFactory = ancestor.lock()->pDWriteFactory;
			dwFactory->CreateTextFormat(
				L"Segoe UI",                // Font family name.
				nullptr,                       // Font collection (NULL sets it to use the system font collection).
				DWRITE_FONT_WEIGHT_REGULAR,
				DWRITE_FONT_STYLE_NORMAL,
				DWRITE_FONT_STRETCH_NORMAL,
				14.0f,
				L"",
				&text_format);
			text_format->SetTextAlignment(DWRITE_TEXT_ALIGNMENT::DWRITE_TEXT_ALIGNMENT_CENTER);
			text_format->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
		}
	public:
		~dep_widget()
		{
			if (brush)
				brush->Release();
			if (brush_frame)
				brush_frame->Release();
			if (brush_font)
				brush_font->Release();
			if (text_format)
				text_format->Release();
		}

		friend class scene;
	};
	using button = dep_widget<logic_button>;
	static_assert(has_implimented_dep_widget<logic_button>);
#endif

	class logic_rect : virtual public logic_widget
	{
	public:
		color brush_color{};
		color pen_color{};
		real pen_size{};
		logic_rect()
		{
			is_focusable = false;
		}
	};
#if _MSVC_LANG
	template <>
	class dep_widget<logic_rect> : virtual public logic_rect, virtual public dep_widget_base
	{
	public:
		virtual void on_paint() const override
		{
			auto rect = D2D1::RectF(0, 0, cx, cy);
			{
				ID2D1SolidColorBrush* brush;
				pRenderTarget->CreateSolidColorBrush(brush_color, &brush);
				pRenderTarget->FillRectangle(rect, brush);
				brush->Release();
			}
			if (pen_size)
			{
				ID2D1SolidColorBrush* brush;
				pRenderTarget->CreateSolidColorBrush(pen_color, &brush);
				pRenderTarget->DrawRectangle(rect, brush, pen_size);
				brush->Release();
			}
		}
	};
	using rect = dep_widget<logic_rect>;
	static_assert(has_implimented_dep_widget<logic_rect>);
#endif
}