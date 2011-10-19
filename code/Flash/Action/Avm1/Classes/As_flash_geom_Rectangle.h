#ifndef traktor_flash_As_flash_geom_Rectangle_H
#define traktor_flash_As_flash_geom_Rectangle_H

#include "Flash/Action/Avm1/ActionClass.h"

namespace traktor
{
	namespace flash
	{

class Point;
class Rectangle;

/*! \brief Rectangle class.
 * \ingroup Flash
 */
class As_flash_geom_Rectangle : public ActionClass
{
	T_RTTI_CLASS;

public:
	As_flash_geom_Rectangle(ActionContext* context);

	virtual void init(ActionObject* self, const ActionValueArray& args) const;

	virtual void coerce(ActionObject* self) const;

private:
	Ref< Rectangle > Rectangle_clone(const Rectangle* self) const;

	bool Rectangle_contains(const Rectangle* self, avm_number_t x, avm_number_t y) const;

	bool Rectangle_containsPoint(const Rectangle* self, const Point* pt) const;

	bool Rectangle_containsRectangle(const Rectangle* self, const Rectangle* rc) const;

	bool Rectangle_equals(const Rectangle* self, const Rectangle* rc) const;

	void Rectangle_inflate(Rectangle* self, avm_number_t dx, avm_number_t dy) const;

	void Rectangle_inflatePoint(Rectangle* self, const Point* pt) const;

	void Rectangle_intersection(const Rectangle* self) const;

	void Rectangle_intersects(const Rectangle* self) const;

	bool Rectangle_isEmpty(const Rectangle* self) const;

	void Rectangle_offset(Rectangle* self, avm_number_t dx, avm_number_t dy) const;

	void Rectangle_offsetPoint(Rectangle* self, const Point* pt) const;

	void Rectangle_setEmpty(Rectangle* self) const;

	ActionValue Rectangle_toString(const Rectangle* self) const;

	void Rectangle_union(const Rectangle* self) const;

	avm_number_t Rectangle_get_bottom(const Rectangle* self) const;

	void Rectangle_set_bottom(Rectangle* self, avm_number_t bottom) const;

	Ref< Point > Rectangle_get_bottomRight(const Rectangle* self) const;

	void Rectangle_set_bottomRight(Rectangle* self, const Point* pt) const;

	avm_number_t Rectangle_get_height(const Rectangle* self) const;

	void Rectangle_set_height(Rectangle* self, avm_number_t height) const;

	avm_number_t Rectangle_get_left(const Rectangle* self) const;

	void Rectangle_set_left(Rectangle* self, avm_number_t left) const;

	avm_number_t Rectangle_get_right(const Rectangle* self) const;

	void Rectangle_set_right(Rectangle* self, avm_number_t right) const;

	Ref< Point > Rectangle_get_size(const Rectangle* self) const;

	void Rectangle_set_size(Rectangle* self, const Point* pt) const;

	avm_number_t Rectangle_get_top(const Rectangle* self) const;

	void Rectangle_set_top(Rectangle* self, avm_number_t top) const;

	Ref< Point > Rectangle_get_topLeft(const Rectangle* self) const;

	void Rectangle_set_topLeft(Rectangle* self, const Point* pt) const;

	avm_number_t Rectangle_get_width(const Rectangle* self) const;

	void Rectangle_set_width(Rectangle* self, avm_number_t width) const;

	avm_number_t Rectangle_get_x(const Rectangle* self) const;

	void Rectangle_set_x(Rectangle* self, avm_number_t x) const;

	avm_number_t Rectangle_get_y(const Rectangle* self) const;

	void Rectangle_set_y(Rectangle* self, avm_number_t y) const;
};

	}
}

#endif	// traktor_flash_As_flash_geom_Rectangle_H
