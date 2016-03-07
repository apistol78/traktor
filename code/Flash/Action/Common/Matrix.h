#ifndef traktor_flash_Matrix_H
#define traktor_flash_Matrix_H

#include "Core/Math/Matrix33.h"
#include "Flash/Action/ActionObjectRelay.h"

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

class Point;

/*! \brief ActionScript Matrix class.
 * \ingroup Flash
 */
class T_DLLCLASS Matrix : public ActionObjectRelay
{
	T_RTTI_CLASS;

public:
	Matrix33 m_v;

	Matrix();

	Matrix(const avm_number_t v[6]);

	Ref< Matrix > clone();

	void concat(const Matrix* rh);

	void createBox(avm_number_t scaleX, avm_number_t scaleY, avm_number_t rotation, avm_number_t tx, avm_number_t ty);

	void createGradientBox(avm_number_t width, avm_number_t height, avm_number_t rotation, avm_number_t tx, avm_number_t ty);

	Ref< Point > deltaTransformPoint(const Point* pt);

	void identity();

	void invert();

	void rotate(avm_number_t angle);

	void scale(avm_number_t scaleX, avm_number_t scaleY);

	std::wstring toString();

	Ref< Point > transformPoint(const Point* pt);

	void translate(avm_number_t x, avm_number_t y);
};

	}
}

#endif	// traktor_flash_Matrix_H
