#ifndef traktor_flash_Rectangle_H
#define traktor_flash_Rectangle_H

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

/*! \brief ActionScript number wrapper.
 * \ingroup Flash
 */
class T_DLLCLASS Rectangle : public ActionObjectRelay
{
	T_RTTI_CLASS;

public:
	avm_number_t m_x;
	avm_number_t m_y;
	avm_number_t m_width;
	avm_number_t m_height;

	Rectangle();

	Rectangle(const avm_number_t v[4]);

	Ref< Rectangle > clone(const Rectangle* s);

	bool contains(avm_number_t x, avm_number_t y);

	bool containsPoint(const Point* pt);

	bool containsRectangle(const Rectangle* r);

	bool equals(const Rectangle* s);

	void inflate(avm_number_t x, avm_number_t y);

	void inflatePoint(const Point* pt);

	void intersection();

	void intersects();

	bool isEmpty();

	void offset(avm_number_t x, avm_number_t y);

	void offsetPoint(const Point* pt);

	void setEmpty();

	std::wstring toString();

	void union_();

	avm_number_t getBottom();

	void setBottom(avm_number_t v);

	Ref< Point > getBottomRight();

	void setBottomRight(const Point* pt);

	avm_number_t getLeft();

	void setLeft(avm_number_t v);

	avm_number_t getRight();

	void setRight(avm_number_t v);

	Ref< Point > getSize();

	void setSize(const Point* pt);

	avm_number_t getTop();

	void setTop(avm_number_t v);

	Ref< Point > getTopLeft();

	void setTopLeft(const Point* pt);

	virtual bool setMember(ActionContext* context, uint32_t memberName, const ActionValue& memberValue) T_OVERRIDE T_FINAL;

	virtual bool getMember(ActionContext* context, uint32_t memberName, ActionValue& outMemberValue) T_OVERRIDE T_FINAL;
};

	}
}

#endif	// traktor_flash_Rectangle_H
