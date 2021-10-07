#include <algorithm>
#include "Core/Class/CastAny.h"
#include "Spark/Mouse.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.Mouse", Mouse, Object)

Mouse::Mouse()
{
	//setMember("show", ActionValue(createNativeFunction(context, this, &Mouse::Mouse_show)));
	//setMember("hide", ActionValue(createNativeFunction(context, this, &Mouse::Mouse_hide)));
}

void Mouse::eventMouseDown(int x, int y, int button)
{
	//ActionValue broadcastMessageValue;
	//getMember("broadcastMessage", broadcastMessageValue);

	//Ref< ActionFunction > broadcastMessageFn = broadcastMessageValue.getObject< ActionFunction >();
	//if (broadcastMessageFn)
	//{
	//	ActionValueArray args(getContext()->getPool(), 1);
	//	args[0] = ActionValue("onButtonDown");
	//	broadcastMessageFn->call(this, args);
	//}

	m_eventButtonDown.issue();
}

void Mouse::eventMouseUp(int x, int y, int button)
{
	//ActionValue broadcastMessageValue;
	//getMember("broadcastMessage", broadcastMessageValue);

	//Ref< ActionFunction > broadcastMessageFn = broadcastMessageValue.getObject< ActionFunction >();
	//if (broadcastMessageFn)
	//{
	//	ActionValueArray args(getContext()->getPool(), 1);
	//	args[0] = ActionValue("onButtonUp");
	//	broadcastMessageFn->call(this, args);
	//}

	m_eventButtonUp.issue();
}

void Mouse::eventMouseMove(int x, int y, int button)
{
	//ActionValue broadcastMessageValue;
	//getMember("broadcastMessage", broadcastMessageValue);

	//Ref< ActionFunction > broadcastMessageFn = broadcastMessageValue.getObject< ActionFunction >();
	//if (broadcastMessageFn)
	//{
	//	ActionValueArray args(getContext()->getPool(), 1);
	//	args[0] = ActionValue("onMouseMove");
	//	broadcastMessageFn->call(this, args);
	//}

	m_eventMouseMove.issue();
}

void Mouse::eventMouseWheel(int x, int y, int delta)
{
	//ActionValue broadcastMessageValue;
	//getMember("broadcastMessage", broadcastMessageValue);

	//Ref< ActionFunction > broadcastMessageFn = broadcastMessageValue.getObject< ActionFunction >();
	//if (broadcastMessageFn)
	//{
	//	ActionValueArray args(getContext()->getPool(), 2);
	//	args[0] = ActionValue("onMouseWheel");
	//	args[1] = ActionValue(delta);
	//	broadcastMessageFn->call(this, args);
	//}

	Any argv = CastAny< int >::set(delta);
	m_eventMouseWheel.issue(1, &argv);
}

	}
}
