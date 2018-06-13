#include "Core/Class/IRuntimeClass.h"
#include "Core/Serialization/ISerializer.h"
#include "Resource/Member.h"
#include "UiKit/Editor/WidgetScaffolding.h"

namespace traktor
{
	namespace uikit
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.uikit.WidgetScaffolding", 0, WidgetScaffolding, ISerializable)

void WidgetScaffolding::serialize(ISerializer& s)
{
	s >> resource::Member< IRuntimeClass >(L"scaffoldingClass", m_scaffoldingClass);
}

	}
}