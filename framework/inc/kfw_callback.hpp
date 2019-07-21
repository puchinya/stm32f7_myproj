/*
 * kfw_callback.hpp
 *
 *  Created on: 2019/03/14
 *      Author: masatakanabeshima
 */

#pragma once
#ifndef KFW_CALLBACK_HPP
#define KFW_CALLBACK_HPP

#include "kfw_common.hpp"
#include <new>

namespace kfw
{
	template <typename R, typename ...Args>
	class Callback;

	template <typename R, typename ...Args>
	class Callback<R(Args...)> final
	{
	public:
		Callback(R(*func)(Args...) = nullptr)
		{
			if(func == nullptr) {
				memset(this, 0, sizeof(Callback));
			} else {
				init(func);
			}
		}

		Callback(const Callback<R(Args...)> &obj)
		{
			memset(this, 0, sizeof(Callback));
			if(obj.m_ops != nullptr) {
				obj.m_ops->move(this, &obj);
			}
			m_ops = obj.m_ops;
		}

		template<typename T, typename U>
		Callback(U *obj, R(T::*method)(Args...))
		{
			init(MethodContext<T, R(T::*)(Args...)>(obj, method));
		}

		template<typename T, typename U>
		Callback(const U *obj, R(T::*method)(Args...) const)
		{
			init(MethodContext<const T, R(T::*)(Args...) const>(obj, method));
		}

		template<typename T, typename U>
		Callback(volatile U *obj, R(T::*method)(Args...) volatile)
		{
			init(MethodContext<volatile T, R(T::*)(Args...) volatile>(obj, method));
		}

		template<typename T, typename U>
		Callback(const U *obj, R(T::*method)(Args...) const volatile)
		{
			init(MethodContext<const volatile T, R(T::*)(Args...) const volatile>(obj, method));
		}

		R call(Args... args)
		{
			return m_ops->call(this, args...);
		}

		R operator()(Args... args)
		{
			return call(args...);
		}

		operator bool() const
		{
			return m_ops != nullptr;
		}

		Callback<R(Args...)> &operator = (const Callback<R(Args...)> &obj)
		{
			memcpy(this, &obj, sizeof(*this));
			return *this;
		}

	private:

		struct DummyClass;
		volatile DummyClass *m_obj;

		volatile union {
			R (*static_func)(Args...);
			R (DummyClass::*method_func)(Args...);
			uint32_t dummy[4];
		} m_func;

		volatile const struct Ops {
			R(*call)(const void *, Args...);
			void (*move)(void *, const void *);
			void (*dtor)(void *);
		} *m_ops;

		template <typename F>
		void init(const F &f)
		{
			static const Ops ops = {
				&Callback::function_call<F>,
				&Callback::function_move<F>,
				&Callback::function_dtor<F>
			};

			static_assert(sizeof(Callback) - sizeof(Ops) >= sizeof(F),
					"Type F must not exceed the size of the Callback class");

			memset(this, 0, sizeof(Callback));
			new (this) F(f);
			m_ops = &ops;
		}

		template <typename F>
		static R function_call(const void *p)
		{
			return (*(F *)p)();
		}

		template <typename F>
		static void function_move(void *d, const void *p)
		{
			new (d) F(*(F *)p);
		}

		template <typename F>
		static void function_dtor(void *p)
		{
			((F *)p)->~F();
		}

		template <typename O, typename M>
		struct MethodContext {
			O *m_obj;
			M m_method;

			MethodContext(O *obj, M method)
			: m_obj(obj), m_method(method) {}

			R operator()(Args... a) const
			{
				return (m_obj->*m_method)(a...);
			}
		};

		template <typename F, typename A>
		struct FunctionContext {
			volatile A *m_arg;
			volatile F m_func;

			FunctionContext(F func, A *arg)
			: m_arg(arg), m_func(func)
			{

			}

			R operator()(Args... a) const
			{
				return m_func(m_arg, a...);
			}
		};
	};
};

#endif /* KFW_CALLBACK_HPP_ */
