#ifndef traktor_flash_As_flash_filters_BitmapFilter_H
#define traktor_flash_As_flash_filters_BitmapFilter_H

#include "Flash/Action/ActionClass.h"

namespace traktor
{
	namespace flash
	{

/*! \brief BitmapFilter class.
 * \ingroup Flash
 */
class As_flash_filters_BitmapFilter : public ActionClass
{
	T_RTTI_CLASS;

public:
	As_flash_filters_BitmapFilter(ActionContext* context);

	virtual void initialize(ActionObject* self) T_OVERRIDE T_FINAL;

	virtual void construct(ActionObject* self, const ActionValueArray& args) T_OVERRIDE T_FINAL;

	virtual ActionValue xplicit(const ActionValueArray& args) T_OVERRIDE T_FINAL;
};

	}
}

#endif	// traktor_flash_As_flash_filters_BitmapFilter_H
