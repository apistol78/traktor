#ifndef traktor_ui_EventSubject_H
#define traktor_ui_EventSubject_H

#include <map>
#include <vector>
#include "Core/Object.h"
#include "Core/RefArray.h"
#include "Ui/Enums.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{

class Event;
class EventHandler;

/*! \brief Event subject.
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

		FunctionEventHandler(function_t fn)
		:	m_fn(fn)
		{
		}

		virtual void notify(Event* event) T_OVERRIDE
		{
			(*m_fn)(
				checked_type_cast< EventType*, false >(event)
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

		MethodEventHandler(ClassType* target, method_t method)
		:	m_target(target)
		,	m_method(method)
		{
		}

		virtual void notify(Event* event) T_OVERRIDE
		{
			(m_target->*m_method)(
				checked_type_cast< EventType*, false >(event)
			);
		}

	private:
		Ref< ClassType > m_target;
		method_t m_method;
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
		Ref< IEventHandler > eventHandler = new MethodEventHandler< ClassType, EventType >(target, method);
		addEventHandler(type_of< EventType >(), eventHandler);
		return eventHandler;
	}

	template < typename EventType >
	IEventHandler* addEventHandler(typename FunctionEventHandler< EventType >::function_t fn)
	{
		Ref< IEventHandler > eventHandler = new FunctionEventHandler< EventType >(fn);
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

protected:
	void removeAllEventHandlers();

private:
	typedef RefArray< IEventHandler > EventHandlers;
	std::map< const TypeInfo*, std::vector< EventHandlers > > m_eventHandlers;
};

	}
}

#endif	// traktor_ui_EventSubject_H
