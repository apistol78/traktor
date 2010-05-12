#include "Flash/Action/Classes/ColorTransform.h"
#include "Flash/Action/Avm1/Classes/As_flash_geom_ColorTransform.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.ColorTransform", ColorTransform, ActionObject)

ColorTransform::ColorTransform(const SwfCxTransform& transform)
:	ActionObject(As_flash_geom_ColorTransform::getInstance())
,	m_transform(transform)
{
}

SwfCxTransform& ColorTransform::getTransform()
{
	return m_transform;
}

const SwfCxTransform& ColorTransform::getTransform() const
{
	return m_transform;
}

	}
}
