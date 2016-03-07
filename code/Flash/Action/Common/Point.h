#ifndef traktor_flash_Point_H
#define traktor_flash_Point_H

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

/*! \brief ActionScript number wrapper.
 * \ingroup Flash
 */
class T_DLLCLASS Point : public ActionObjectRelay
{
	T_RTTI_CLASS;

public:
	avm_number_t m_x;
	avm_number_t m_y;

	Point();

	Point(avm_number_t x, avm_number_t y);

	static avm_number_t distance(const Point& pt1, const Point& pt2);

	static Ref< Point > interpolate(const Point& pt1, const Point& pt2, avm_number_t f);

	static Ref< Point > polar(avm_number_t length, avm_number_t angle);

	Ref< Point > add_1(const Point* pt);

	Ref< Point > add_2(avm_number_t x, avm_number_t y);

	Ref< Point > clone(const Point* pt);

	bool equals(const Point* pt);

	void normalize();

	void offset(avm_number_t x, avm_number_t y);

	void subtract(avm_number_t x, avm_number_t y);

	std::wstring toString();

	avm_number_t getLength();

	void setLength(avm_number_t length);

	virtual bool setMember(ActionContext* context, uint32_t memberName, const ActionValue& memberValue) T_OVERRIDE T_FINAL;

	virtual bool getMember(ActionContext* context, uint32_t memberName, ActionValue& outMemberValue) T_OVERRIDE T_FINAL;
};

	}
}

#endif	// traktor_flash_Point_H
