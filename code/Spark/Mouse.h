#pragma once

#include "Spark/Event.h"

namespace traktor
{
	namespace spark
	{

/*! Mouse class.
 * \ingroup Spark
 */
class Mouse : public Object
{
	T_RTTI_CLASS;

public:
	Mouse();

	void eventMouseDown(int x, int y, int button);

	void eventMouseUp(int x, int y, int button);

	void eventMouseMove(int x, int y, int button);

	void eventMouseWheel(int x, int y, int delta);

	/*! \group Events */
	//@{

	Event* getEventButtonDown() { return &m_eventButtonDown; }

	Event* getEventButtonUp() { return &m_eventButtonUp; }

	Event* getEventMouseMove() { return &m_eventMouseMove; }

	Event* getEventMouseWheel() { return &m_eventMouseWheel; }

	//@}

private:
	Event m_eventButtonDown;
	Event m_eventButtonUp;
	Event m_eventMouseMove;
	Event m_eventMouseWheel;
};

	}
}

