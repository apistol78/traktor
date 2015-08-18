#ifndef traktor_flash_As_flash_geom_Transform_H
#define traktor_flash_As_flash_geom_Transform_H

#include "Flash/Action/ActionClass.h"

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
	As_flash_geom_Transform(ActionContext* context);

	virtual void initialize(ActionObject* self);

	virtual void construct(ActionObject* self, const ActionValueArray& args);

	virtual ActionValue xplicit(const ActionValueArray& args);

private:
	Ref< ColorTransform > Transform_get_colorTransform(Transform* self) const;

	void Transform_set_colorTransform(Transform* self, const ColorTransform* colorTransform) const;
};

	}
}

#endif	// traktor_flash_As_flash_geom_Transform_H
