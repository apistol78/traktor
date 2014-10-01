#include <algorithm>
#include "Core/System/Android/DelegateInstance.h"

namespace traktor
{

void DelegateInstance::addDelegate(IDelegate* delegate)
{
	m_delegates.push_back(delegate);
}

void DelegateInstance::removeDelegate(IDelegate* delegate)
{
	std::vector< IDelegate* >::iterator i = std::find(m_delegates.begin(), m_delegates.end(), delegate);
	if (i != m_delegates.end())
		m_delegates.erase(i);
}

void DelegateInstance::handleCommand(struct android_app* app, int32_t cmd)
{
	for (std::vector< IDelegate* >::iterator i = m_delegates.begin(); i != m_delegates.end(); ++i)
		(*i)->notifyHandleCommand(app, cmd);
}

void DelegateInstance::handleInput(struct android_app* app, AInputEvent* event)
{
	for (std::vector< IDelegate* >::iterator i = m_delegates.begin(); i != m_delegates.end(); ++i)
		(*i)->notifyHandleInput(app, event);
}

}
