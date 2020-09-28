#pragma once

#pragma once

#include "utils/direct_ui.hpp"

namespace direct_ui
{
	class logic_unpainted_button : virtual public logic_widget
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
			require_update();
		}
		virtual void on_left_up(real x, real y) override
		{
			is_mouse_down--;
			require_update();
			if (is_mouse_hover)
				callback();
		}

		friend class dep_widget<logic_unpainted_button>;
	};
#if _MSVC_LANG
	template <>
	class dep_widget<logic_unpainted_button> : virtual public logic_unpainted_button, virtual public dep_widget_base
	{
	public:
	};
	using unpainted_button = dep_widget<logic_unpainted_button>;
	static_assert(has_implimented_dep_widget<logic_unpainted_button>);
#endif
}