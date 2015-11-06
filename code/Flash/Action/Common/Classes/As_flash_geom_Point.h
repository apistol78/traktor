#ifndef traktor_flash_As_flash_geom_Point_H
#define traktor_flash_As_flash_geom_Point_H

#include "Flash/Action/ActionClass.h"

namespace traktor
{
	namespace flash
	{

struct CallArgs;

/*! \brief Point class.
 * \ingroup Flash
 */
class As_flash_geom_Point : public ActionClass
{
	T_RTTI_CLASS;

public:
	As_flash_geom_Point(ActionContext* context);

	virtual void initialize(ActionObject* self) T_OVERRIDE T_FINAL;

	virtual void construct(ActionObject* self, const ActionValueArray& args) T_OVERRIDE T_FINAL;

	virtual ActionValue xplicit(const ActionValueArray& args) T_OVERRIDE T_FINAL;

private:
	uint32_t m_idX;
	uint32_t m_idY;

	void Point_distance(CallArgs& ca);

	void Point_interpolate(CallArgs& ca);

	void Point_polar(CallArgs& ca);

	void Point_add(CallArgs& ca);

	void Point_clone(CallArgs& ca);

	void Point_equals(CallArgs& ca);

	void Point_normalize(CallArgs& ca);

	void Point_offset(CallArgs& ca);

	void Point_subtract(CallArgs& ca);

	void Point_toString(CallArgs& ca);

	void Point_get_length(CallArgs& ca);

	void Point_set_length(CallArgs& ca);
};

	}
}

#endif	// traktor_flash_As_flash_geom_Point_H
