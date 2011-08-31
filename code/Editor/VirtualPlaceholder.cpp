#include "Editor/VirtualPlaceholder.h"

namespace traktor
{
	namespace editor
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.editor.VirtualPlaceholder", 0, VirtualPlaceholder, ISerializable)

bool VirtualPlaceholder::serialize(ISerializer& s)
{
	return true;
}

	}
}
