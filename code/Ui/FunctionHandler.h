#ifndef traktor_ui_FunctionHandler_H
#define traktor_ui_FunctionHandler_H

#include "Ui/EventHandler.h"

namespace traktor
{
	namespace ui
	{

/*! \brief Function event handler.
 * \ingroup UI
 */
class FunctionHandler : public EventHandler
{
public:
	typedef void (*F)(Event* event);

	FunctionHandler(F function) :
		m_function(function)
	{
	}

	virtual void notify(Event* event)
	{
		m_function(event);
	}

private:
	F m_function;
};

/*! \brief Create function event handler.
 * \ingroup UI
 */
inline EventHandler* createFunctionHandler(FunctionHandler::F function)
{
	return gc_new< FunctionHandler >(function);
}

	}
}

#endif	// traktor_ui_FunctionHandler_H
