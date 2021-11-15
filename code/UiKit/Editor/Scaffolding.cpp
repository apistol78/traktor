#include "Core/Class/IRuntimeClass.h"
#include "Core/Serialization/ISerializer.h"
#include "Resource/Member.h"
#include "UiKit/Editor/Scaffolding.h"

namespace traktor
{
	namespace uikit
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.uikit.Scaffolding", 0, Scaffolding, ISerializable)

void Scaffolding::serialize(ISerializer& s)
{
	s >> resource::Member< IRuntimeClass >(L"scaffoldingClass", m_scaffoldingClass);
}

	}
}
