#pragma once

#include "utils/direct_ui.hpp"

namespace direct_ui
{
	class logic_vessel : virtual public logic_group
	{
	public:
		std::vector<std::weak_ptr<logic_widget>> hover_to_show;

		virtual void on_mouse_hover() override
		{
			logic_group::on_mouse_hover();
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
			for (auto p : hover_to_show)
				if (!p.expired())
				{
					auto t = p.lock();
					t->set_visible(false);
					t->require_update();
				}
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