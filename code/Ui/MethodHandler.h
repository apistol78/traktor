#ifndef traktor_ui_MethodHandler_H
#define traktor_ui_MethodHandler_H

#include "Core/Heap/Ref.h"
#include "Core/Heap/HeapNew.h"
#include "Ui/EventHandler.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{

class EventSubject;

/*! \brief Class method event handler.
 * \ingroup UI
 */
template < typename T >
class MethodHandler : public EventHandler
{
public:
	typedef void (T::*M)(Event* event);

	MethodHandler< T >(T* object, M method) :
		m_object(object),
		m_method(method)
	{
	}

	virtual void notify(Event* event)
	{
		(m_object->*m_method)(event);
	}

private:
	T* m_object;
	M m_method;
};

/*! \brief Create method event handler.
 * \ingroup UI
 */
template < typename T >
inline EventHandler* createMethodHandler(T* object, typename MethodHandler< T >::M method)
{
	return gc_new< MethodHandler< T > >(object, method);
}

	}
}

#endif	// traktor_ui_MethodHandler_H
