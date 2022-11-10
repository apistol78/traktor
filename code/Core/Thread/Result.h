/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <functional>
#include "Core/Object.h"
#include "Core/Ref.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! Deferred result.
 *
 * As some operations are expected
 * to take considerable time this
 * class is used to defer results
 * until it's ready.
 *
 * In order to be as simple as possible
 * it's designed to either to used
 * with a callback, polled or blocked.
 *
 * Ref< Result > result = lengthyOperation();
 *
 * // Either defer result as a callback.
 * result->defer(this, &MyClass::deferredResultFinished, ...);
 *
 * // Or poll until result is ready.
 * if (result->ready())
 * {
 * }
 *
 * // Or block.
 * saveGame = result->get< MyData >();
 *
 */
class T_DLLCLASS Result : public Object
{
	T_RTTI_CLASS;

public:
	struct IDeferred : public Object
	{
		virtual void dispatch(const Result& result) const = 0;
	};

	template < typename ClassType >
	class DeferredMethod : public IDeferred
	{
	public:
		typedef void (ClassType::*method_t)(const Result& result);

		DeferredMethod(ClassType* object, method_t method)
		:	m_object(object)
		,	m_method(method)
		{
		}

		virtual void dispatch(const Result& result) const override final
		{
			(m_object->*m_method)(result);
		}

	private:
		ClassType* m_object;
		method_t m_method;
	};

	class DeferredLambda : public IDeferred
	{
	public:
		DeferredLambda(const std::function< void() >& fn)
		:	m_fn(fn)
		{
		}

		virtual void dispatch(const Result& result) const override final
		{
			m_fn();
		}

	private:
		std::function< void() > m_fn;
	};

	Result();

	explicit Result(bool succeed);

	void succeed();

	void fail();

	bool ready() const;

	bool succeeded() const;

	void defer(IDeferred* deferred);

	template < typename ClassType >
	void defer(ClassType* object, typename DeferredMethod< ClassType >::method_t method)
	{
		defer(new DeferredMethod< ClassType >(object, method));
	}

	void defer(const std::function< void() >& fn)
	{
		defer(new DeferredLambda(fn));
	}

protected:
	void wait() const;

private:
	bool m_ready;
	bool m_succeeded;
	Ref< IDeferred > m_deferred;

	void deferred();
};

}
