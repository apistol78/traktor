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

void DelegateInstance::handleCommand(int32_t cmd)
{
	for (std::vector< IDelegate* >::iterator i = m_delegates.begin(); i != m_delegates.end(); ++i)
		(*i)->notifyHandleCommand(this, cmd);
}

void DelegateInstance::handleInput(AInputEvent* event)
{
	for (std::vector< IDelegate* >::iterator i = m_delegates.begin(); i != m_delegates.end(); ++i)
		(*i)->notifyHandleInput(this, event);
}

void DelegateInstance::handleEvents()
{
	for (std::vector< IDelegate* >::iterator i = m_delegates.begin(); i != m_delegates.end(); ++i)
		(*i)->notifyHandleEvents(this);
}

}
