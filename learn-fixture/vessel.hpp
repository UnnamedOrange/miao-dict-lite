#pragma once

#include "utils/direct_ui.hpp"

namespace direct_ui
{
	class logic_vessel : virtual public logic_group
	{
	public:
		std::vector<std::weak_ptr<logic_widget>> hover_to_show;
		std::weak_ptr<logic_widget> inner_group;
	private:
		static constexpr real para_rate = 0.1;
		bool is_mouse_hover{};
		real para_x{}, para_y{};
	private:
		real mouse_x{}, mouse_y{};
	public:
		virtual void on_update(std::chrono::high_resolution_clock::duration elapsed) override
		{
			logic_group::on_update(elapsed);
			auto sec = std::chrono::duration_cast<std::chrono::duration<real>>(elapsed);
			real dt = sec.count();
			{
				real para_x_target = is_mouse_hover * (cx / 2 - mouse_x) * para_rate;
				constexpr real speed = 75;
				real step = (para_x_target > para_x ? 1 : -1) * speed * dt;
				para_x += step;
				if (std::abs(para_x - para_x_target) > speed * dt)
					require_update();
			}
			{
				real para_y_target = is_mouse_hover * (cy / 2 - mouse_y) * para_rate;
				constexpr real speed = 75;
				real step = (para_y_target > para_y ? 1 : -1) * speed * dt;
				para_y += step;
				if (std::abs(para_y - para_y_target) > speed * dt)
					require_update();
			}
			if (!inner_group.expired())
				inner_group.lock()->move(para_x, para_y);
		}
		virtual void on_mouse_hover() override
		{
			logic_group::on_mouse_hover();
			is_mouse_hover = true;
			require_update();
			for (auto p : hover_to_show)
				if (!p.expired())
				{
					auto t = p.lock();
					t->set_visible(true);
					t->require_update();
				}
		}
		virtual void on_mouse_leave() override
		{
			logic_group::on_mouse_leave();
			is_mouse_hover = false;
			require_update();
			for (auto p : hover_to_show)
				if (!p.expired())
				{
					auto t = p.lock();
					t->set_visible(false);
					t->require_update();
				}
		}
		virtual void on_mouse_move(real x, real y) override
		{
			logic_group::on_mouse_move(x, y);
			mouse_x = x;
			mouse_y = y;
			require_update();
		}

		friend class dep_widget<logic_vessel>;
	};
#if _MSVC_LANG
	template <>
	class dep_widget<logic_vessel> : virtual public logic_vessel, virtual public dep_widget<logic_group>
	{
	};
	using vessel = dep_widget<logic_vessel>;
	static_assert(has_implimented_dep_widget<logic_vessel>);
#endif
}