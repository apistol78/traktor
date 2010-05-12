#include "Flash/FlashCharacterInstance.h"
#include "Flash/Action/Classes/ColorTransform.h"
#include "Flash/Action/Classes/Transform.h"
#include "Flash/Action/Avm1/Classes/As_flash_geom_Transform.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.Transform", Transform, ActionObject)

Transform::Transform(FlashCharacterInstance* instance)
:	ActionObject(As_flash_geom_Transform::getInstance())
,	m_instance(instance)
{
}

Ref< ColorTransform > Transform::getColorTransform() const
{
	return new ColorTransform(m_instance->getColorTransform());
}

void Transform::setColorTransform(const ColorTransform* colorTransform)
{
	m_instance->setColorTransform(colorTransform->getTransform());
}

	}
}
