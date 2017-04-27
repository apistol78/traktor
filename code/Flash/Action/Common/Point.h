/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
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
	float m_x;
	float m_y;

	Point();

	Point(float x, float y);

	static float distance(const Point& pt1, const Point& pt2);

	static Ref< Point > interpolate(const Point& pt1, const Point& pt2, float f);

	static Ref< Point > polar(float length, float angle);

	Ref< Point > add_1(const Point* pt);

	Ref< Point > add_2(float x, float y);

	Ref< Point > clone(const Point* pt);

	bool equals(const Point* pt);

	void normalize();

	void offset(float x, float y);

	void subtract(float x, float y);

	std::wstring toString();

	float getLength();

	void setLength(float length);

	virtual bool setMember(ActionContext* context, uint32_t memberName, const ActionValue& memberValue) T_OVERRIDE T_FINAL;

	virtual bool getMember(ActionContext* context, uint32_t memberName, ActionValue& outMemberValue) T_OVERRIDE T_FINAL;
};

	}
}

#endif	// traktor_flash_Point_H
