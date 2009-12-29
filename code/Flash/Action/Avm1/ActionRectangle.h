#ifndef traktor_flash_ActionRectangle_H
#define traktor_flash_ActionRectangle_H

#include "Flash/Action/ActionObject.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_FLASH_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace flash
	{

/*! \brief ActionScript rectangle.
 * \ingroup Flash
 */
class T_DLLCLASS ActionRectangle : public ActionObject
{
	T_RTTI_CLASS;

public:
	double left;
	double top;
	double right;
	double bottom;

	ActionRectangle();

	ActionRectangle(double left_, double top_, double right_, double bottom_);

	bool contains(double x, double y) const;

	void inflate(double dx, double dy);

	bool isEmpty() const;

	void offset(double dx, double dy);
};

	}
}

#endif	// traktor_flash_ActionRectangle_H
