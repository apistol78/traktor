/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <functional>
#include "Core/Object.h"
#include "Core/Ref.h"
#include "Core/RefArray.h"
#include "Core/Containers/SmallMap.h"
#include "Ui/Enums.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::ui
{

class Event;
class EventHandler;

/*! Event subject.
 * \ingroup UI
 */
class T_DLLCLASS EventSubject : public Object
{
	T_RTTI_CLASS;

public:
	struct IEventHandler : public IRefCount
	{
		virtual void notify(Event* event) = 0;
	};

	template < typename EventType >
	class FunctionEventHandler : public RefCountImpl< IEventHandler >
	{
	public:
		typedef void (*function_t)(EventType*);

		explicit FunctionEventHandler(function_t fn)
		:	m_fn(fn)
		{
		}

		virtual void notify(Event* event) override
		{
			(*m_fn)(
				mandatory_non_null_type_cast< EventType* >(event)
			);
		}

	private:
		function_t m_fn;
	};

	template < typename ClassType, typename EventType >
	class MethodEventHandler : public RefCountImpl< IEventHandler >
	{
	public:
		typedef void (ClassType::*method_t)(EventType*);

		explicit MethodEventHandler(ClassType* target, method_t method)
		:	m_target(target)
		,	m_method(method)
		{
		}

		virtual void notify(Event* event) override
		{
			T_ANONYMOUS_VAR(Ref< ClassType >)(m_target);
			(m_target->*m_method)(
				mandatory_non_null_type_cast< EventType* >(event)
			);
		}

	private:
		ClassType* m_target;
		method_t m_method;
	};

	template < typename EventType >
	class LambdaEventHandler : public RefCountImpl< IEventHandler >
	{
	public:
		explicit LambdaEventHandler(const std::function< void( EventType* ) >& fn)
		:	m_fn(fn)
		{
		}

		virtual void notify(Event* event) override
		{
			m_fn(
				mandatory_non_null_type_cast< EventType* >(event)
			);
		}

	private:
		std::function< void( EventType* ) > m_fn;
	};

	void raiseEvent(Event* event);

	void addEventHandler(const TypeInfo& eventType, IEventHandler* eventHandler);

	template < typename EventType >
	void addEventHandler(IEventHandler* eventHandler)
	{
		addEventHandler(type_of< EventType >(), eventHandler);
	}

	template < typename EventType, typename ClassType >
	IEventHandler* addEventHandler(ClassType* target, typename MethodEventHandler< ClassType, EventType >::method_t method)
	{
		IEventHandler* eventHandler = new MethodEventHandler< ClassType, EventType >(target, method);
		addEventHandler(type_of< EventType >(), eventHandler);
		return eventHandler;
	}

	template < typename EventType >
	IEventHandler* addEventHandler(typename FunctionEventHandler< EventType >::function_t fn)
	{
		IEventHandler* eventHandler = new FunctionEventHandler< EventType >(fn);
		addEventHandler(type_of< EventType >(), eventHandler);
		return eventHandler;
	}

	template < typename EventType >
	IEventHandler* addEventHandler(const std::function< void(EventType*) >& fn)
	{
		IEventHandler* eventHandler = new LambdaEventHandler< EventType >(fn);
		addEventHandler(type_of< EventType >(), eventHandler);
		return eventHandler;
	}

	void removeEventHandler(const TypeInfo& eventType, IEventHandler* eventHandler);

	template < typename EventType >
	void removeEventHandler(IEventHandler* eventHandler)
	{
		removeEventHandler(type_of< EventType >(), eventHandler);
	}

	bool hasEventHandler(const TypeInfo& eventType);

	template < typename EventType >
	bool hasEventHandler()
	{
		return hasEventHandler(type_of< EventType >());
	}

	void enableEventHandlers(const TypeInfo& eventType);

	template < typename EventType >
	void enableEventHandlers()
	{
		enableEventHandlers(type_of< EventType >());
	}

	void disableEventHandlers(const TypeInfo& eventType);

	template < typename EventType >
	void disableEventHandlers()
	{
		disableEventHandlers(type_of< EventType >());
	}

protected:
	void removeAllEventHandlers();

private:
	struct HandlerEntry
	{
		int32_t disableCounter = 0;
		RefArray< IEventHandler > handlers;
	};

	SmallMap< const TypeInfo*, HandlerEntry > m_eventHandlers;
};

}
