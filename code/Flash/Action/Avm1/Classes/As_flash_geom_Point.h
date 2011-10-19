#ifndef traktor_flash_As_flash_geom_Point_H
#define traktor_flash_As_flash_geom_Point_H

#include "Flash/Action/Avm1/ActionClass.h"

namespace traktor
{
	namespace flash
	{

class Point;

/*! \brief Point class.
 * \ingroup Flash
 */
class As_flash_geom_Point : public ActionClass
{
	T_RTTI_CLASS;

public:
	As_flash_geom_Point();

	virtual Ref< ActionObject > alloc(ActionContext* context);

	virtual void init(ActionContext* context, ActionObject* self, const ActionValueArray& args);

private:
	void Point_add(Point* self, const Point* arg) const;

	Ref< Point > Point_clone(const Point* self) const;

	avm_number_t Point_distance(const Point* pt1, const Point* pt2) const;

	bool Point_equals(const Point* self, const Point* pt) const;

	Ref< Point > Point_interpolate(Point* self, const Point* pt1, const Point* pt2, avm_number_t f) const;

	void Point_normalize(Point* self, avm_number_t scale) const;

	void Point_offset(Point* self, avm_number_t dx, avm_number_t dy) const;

	Ref< Point > Point_polar(Point* self, avm_number_t length, avm_number_t angle) const;

	void Point_subtract(Point* self, const Point* pr) const;

	ActionValue Point_toString(const Point* self) const;

	avm_number_t Point_get_length(const Point* self) const;

	void Point_set_length(Point* self, avm_number_t length) const;

	avm_number_t Point_get_x(const Point* self) const;

	void Point_set_x(Point* self, avm_number_t x) const;

	avm_number_t Point_get_y(const Point* self) const;

	void Point_set_y(Point* self, avm_number_t y) const;
};

	}
}

#endif	// traktor_flash_As_flash_geom_Point_H
