#include "Render/Editor/Shader/PinType.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.PinType", PinType, Object)

PinType::PinType()
:	m_class(PtcVoid)
,	m_width(0)
{
}

PinType::PinType(PinTypeClass class_, int32_t width)
:	m_class(class_)
,	m_width(width)
{
}

PinTypeClass PinType::getClass() const
{
	return m_class;
}

int32_t PinType::getWidth() const
{
	return m_width;
}

PinType PinType::maxPrecedence(const PinType& type1, const PinType& type2)
{
	if (type1.getClass() > type2.getClass())
		return type1;
	else if (type2.getClass() > type1.getClass())
		return type2;
	else if (type1.getWidth() > type2.getWidth())
		return type1;
	else
		return type2;
}

	}
}
