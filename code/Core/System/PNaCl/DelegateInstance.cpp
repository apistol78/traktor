#include <algorithm>
#include "Core/System/PNaCl/DelegateInstance.h"

namespace traktor
{

DelegateInstance::DelegateInstance(PP_Instance instance)
:	pp::Instance(instance)
{
}

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

void DelegateInstance::DidChangeView(const pp::View& view)
{
	for (std::vector< IDelegate* >::iterator i = m_delegates.begin(); i != m_delegates.end(); ++i)
		(*i)->notifyDidChangeView(view);

	pp::Instance::DidChangeView(view);
}

void DelegateInstance::DidChangeView(const pp::Rect& position, const pp::Rect& clip)
{
	for (std::vector< IDelegate* >::iterator i = m_delegates.begin(); i != m_delegates.end(); ++i)
		(*i)->notifyDidChangeView(position, clip);

	pp::Instance::DidChangeView(position, clip);
}

void DelegateInstance::DidChangeFocus(bool has_focus)
{
	for (std::vector< IDelegate* >::iterator i = m_delegates.begin(); i != m_delegates.end(); ++i)
		(*i)->notifyDidChangeFocus(has_focus);

	pp::Instance::DidChangeFocus(has_focus);
}

bool DelegateInstance::HandleInputEvent(const pp::InputEvent& event)
{
	for (std::vector< IDelegate* >::iterator i = m_delegates.begin(); i != m_delegates.end(); ++i)
		(*i)->notifyHandleInputEvent(event);

	return pp::Instance::HandleInputEvent(event);
}

bool DelegateInstance::HandleDocumentLoad(const pp::URLLoader& url_loader)
{
	for (std::vector< IDelegate* >::iterator i = m_delegates.begin(); i != m_delegates.end(); ++i)
		(*i)->notifyHandleDocumentLoad(url_loader);

	return pp::Instance::HandleDocumentLoad(url_loader);
}

void DelegateInstance::HandleMessage(const pp::Var& message)
{
	for (std::vector< IDelegate* >::iterator i = m_delegates.begin(); i != m_delegates.end(); ++i)
		(*i)->notifyHandleMessage(message);

	pp::Instance::HandleMessage(message);
}

}
