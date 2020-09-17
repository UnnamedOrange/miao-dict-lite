#pragma once

#include <mutex>

template <typename T, typename mutex_t = std::mutex>
class lock_view
{
	T& ref;
	mutex_t& mutex;

public:
	lock_view() = delete;
	lock_view(const lock_view&) = delete;
	lock_view(lock_view&&) = default;
	lock_view& operator=(const lock_view&) = delete;
	lock_view& operator=(lock_view&&) = default;
	lock_view(T& ref, mutex_t& mutex) : ref(ref), mutex(mutex)
	{
		mutex.lock();
	}
	~lock_view()
	{
		mutex.unlock();
	}

	/// <summary>
	/// 访问对应对象的属性。
	/// </summary>
	T* operator->()
	{
		return &ref;
	}
	/// <summary>
	/// 返回对应对象的引用。
	/// </summary>
	T& operator*()
	{
		return ref;
	}
};
template <typename T, typename mutex_t = std::mutex>
class lock_view_factory
{
	mutex_t mutex;
public:
	lock_view<T, mutex_t> make_lock_view(T& ref)
	{
		return lock_view<T, mutex_t>(ref, mutex);
	}
};
template <typename T>
class lockfree
{
	T origin;
	lock_view_factory<T> lvf;
public:
	template <typename ...Args>
	lockfree(Args&& ...args) : origin(args...) {}
	~lockfree() = default;
	lockfree(const lockfree& another)
	{
		*lvf.make_lock_view(origin) = *another.lvf.make_lock_view(another.origin);
	}
	lockfree(lockfree&& another)
	{
		*lvf.make_lock_view(origin) = std::move(*another.lvf.make_lock_view(another.origin));
	}
	lockfree& operator=(const lockfree& another)
	{
		if (this != &another)
			*lvf.make_lock_view(origin) = *another.lvf.make_lock_view(another.origin);
		return *this;
	}
	lockfree& operator=(lockfree&& another)
	{
		if (this != &another)
			*lvf.make_lock_view(origin) = std::move(*another.lvf.make_lock_view(another.origin));
		return *this;
	}

	lock_view<T> view()
	{
		return lvf.make_lock_view(origin);
	}
	const lock_view<T> view() const
	{
		return lvf.make_lock_view(origin);
	}
};