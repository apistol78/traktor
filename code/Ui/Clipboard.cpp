#include "Ui/Clipboard.h"
#include "Core/Heap/HeapNew.h"
#include "Core/Serialization/DeepClone.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.Clipboard", Clipboard, Object)

void Clipboard::setObject(Serializable* object)
{
	m_clone = gc_new< DeepClone >(object);
}

Serializable* Clipboard::getObject()
{
	return m_clone ? m_clone->create() : 0;
}

	}
}
