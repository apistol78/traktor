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
	auto it = std::find(m_delegates.begin(), m_delegates.end(), delegate);
	if (it != m_delegates.end())
		m_delegates.erase(it);
}

void DelegateInstance::handleCommand(int32_t cmd)
{
	for (auto delegate : m_delegates)
		delegate->notifyHandleCommand(this, cmd);
}

void DelegateInstance::handleInput(AInputEvent* event)
{
	for (auto delegate : m_delegates)
		delegate->notifyHandleInput(this, event);
}

void DelegateInstance::handleEvents()
{
	for (auto delegate : m_delegates)
		delegate->notifyHandleEvents(this);
}

}
