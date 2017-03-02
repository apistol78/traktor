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
	float m_x;
	float m_y;
	float m_width;
	float m_height;

	Rectangle();

	Rectangle(const float v[4]);

	Ref< Rectangle > clone(const Rectangle* s);

	bool contains(float x, float y);

	bool containsPoint(const Point* pt);

	bool containsRectangle(const Rectangle* r);

	bool equals(const Rectangle* s);

	void inflate(float x, float y);

	void inflatePoint(const Point* pt);

	void intersection();

	void intersects();

	bool isEmpty();

	void offset(float x, float y);

	void offsetPoint(const Point* pt);

	void setEmpty();

	std::wstring toString();

	void union_();

	float getBottom();

	void setBottom(float v);

	Ref< Point > getBottomRight();

	void setBottomRight(const Point* pt);

	float getLeft();

	void setLeft(float v);

	float getRight();

	void setRight(float v);

	Ref< Point > getSize();

	void setSize(const Point* pt);

	float getTop();

	void setTop(float v);

	Ref< Point > getTopLeft();

	void setTopLeft(const Point* pt);

	virtual bool setMember(ActionContext* context, uint32_t memberName, const ActionValue& memberValue) T_OVERRIDE T_FINAL;

	virtual bool getMember(ActionContext* context, uint32_t memberName, ActionValue& outMemberValue) T_OVERRIDE T_FINAL;
};

	}
}

#endif	// traktor_flash_Rectangle_H
