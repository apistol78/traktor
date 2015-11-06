#ifndef traktor_flash_As_flash_display_BitmapData_H
#define traktor_flash_As_flash_display_BitmapData_H

#include "Flash/Action/ActionClass.h"

namespace traktor
{
	namespace flash
	{

class BitmapData;

class As_flash_display_BitmapData : public ActionClass
{
	T_RTTI_CLASS;

public:
	As_flash_display_BitmapData(ActionContext* context);

	virtual void initialize(ActionObject* self) T_OVERRIDE T_FINAL;

	virtual void construct(ActionObject* self, const ActionValueArray& args) T_OVERRIDE T_FINAL;

	virtual ActionValue xplicit(const ActionValueArray& args) T_OVERRIDE T_FINAL;

private:
	avm_number_t BitmapData_get_height(const BitmapData* self) const;

	ActionValue BitmapData_get_rectangle(const BitmapData* self) const;

	bool BitmapData_get_transparent(const BitmapData* self) const;

	avm_number_t BitmapData_get_width(const BitmapData* self) const;
};

	}
}

#endif	// traktor_flash_As_flash_display_BitmapData_H
