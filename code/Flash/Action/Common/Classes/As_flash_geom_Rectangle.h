#ifndef traktor_flash_As_flash_geom_Rectangle_H
#define traktor_flash_As_flash_geom_Rectangle_H

#include "Flash/Action/ActionClass.h"

namespace traktor
{
	namespace flash
	{

struct CallArgs;

/*! \brief Rectangle class.
 * \ingroup Flash
 */
class As_flash_geom_Rectangle : public ActionClass
{
	T_RTTI_CLASS;

public:
	As_flash_geom_Rectangle(ActionContext* context);

	virtual void initialize(ActionObject* self);

	virtual void construct(ActionObject* self, const ActionValueArray& args);

	virtual ActionValue xplicit(const ActionValueArray& args);

private:
	uint32_t m_idX;
	uint32_t m_idY;
	uint32_t m_idWidth;
	uint32_t m_idHeight;

	void Rectangle_clone(CallArgs& ca);

	void Rectangle_contains(CallArgs& ca);

	void Rectangle_containsPoint(CallArgs& ca);

	void Rectangle_containsRectangle(CallArgs& ca);

	void Rectangle_equals(CallArgs& ca);

	void Rectangle_inflate(CallArgs& ca);

	void Rectangle_inflatePoint(CallArgs& ca);

	void Rectangle_intersection(CallArgs& ca);

	void Rectangle_intersects(CallArgs& ca);

	void Rectangle_isEmpty(CallArgs& ca);

	void Rectangle_offset(CallArgs& ca);

	void Rectangle_offsetPoint(CallArgs& ca);

	void Rectangle_setEmpty(CallArgs& ca);

	void Rectangle_toString(CallArgs& ca);

	void Rectangle_union(CallArgs& ca);

	void Rectangle_get_bottom(CallArgs& ca);

	void Rectangle_set_bottom(CallArgs& ca);

	void Rectangle_get_bottomRight(CallArgs& ca);

	void Rectangle_set_bottomRight(CallArgs& ca);

	void Rectangle_get_left(CallArgs& ca);

	void Rectangle_set_left(CallArgs& ca);

	void Rectangle_get_right(CallArgs& ca);

	void Rectangle_set_right(CallArgs& ca);

	void Rectangle_get_size(CallArgs& ca);

	void Rectangle_set_size(CallArgs& ca);

	void Rectangle_get_top(CallArgs& ca);

	void Rectangle_set_top(CallArgs& ca);

	void Rectangle_get_topLeft(CallArgs& ca);

	void Rectangle_set_topLeft(CallArgs& ca);

	void getRectangleValues(ActionObject* self, ActionValue rc[4]);

	void setRectangleValues(ActionObject* self, ActionValue rc[4]);

	bool getSize(ActionObject* self, avm_number_t sz[2]);

	bool getRectangle(ActionObject* self, avm_number_t rc[4]);

	void setRectangle(ActionObject* self, avm_number_t rc[4]);
};

	}
}

#endif	// traktor_flash_As_flash_geom_Rectangle_H
