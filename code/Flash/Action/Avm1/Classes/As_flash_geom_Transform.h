#ifndef traktor_flash_As_flash_geom_Transform_H
#define traktor_flash_As_flash_geom_Transform_H

#include "Flash/Action/Avm1/ActionClass.h"

namespace traktor
{
	namespace flash
	{

class ColorTransform;
class Transform;

class As_flash_geom_Transform : public ActionClass
{
	T_RTTI_CLASS;

public:
	As_flash_geom_Transform();

private:
	virtual ActionValue construct(ActionContext* context, const ActionValueArray& args);

	Ref< ColorTransform > Transform_get_colorTransform(Transform* self) const;

	void Transform_set_colorTransform(Transform* self, const ColorTransform* colorTransform) const;
};

	}
}

#endif	// traktor_flash_As_flash_geom_Transform_H
