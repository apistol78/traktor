#include "Shape/Editor/Solid/SolidEntityData.h"

namespace traktor
{
    namespace shape
    {

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.shape.SolidEntityData", 0, SolidEntityData, world::GroupEntityData)

void SolidEntityData::serialize(ISerializer& s)
{
	world::GroupEntityData::serialize(s);
}

    }
}