/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_spark_IComponent_H
#define traktor_spark_IComponent_H

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPARK_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class Vector2;

	namespace spark
	{

/*! \brief Character component instance.
 * \ingroup Spark
 */
class T_DLLCLASS IComponent : public Object
{
	T_RTTI_CLASS;

public:
	virtual void update() = 0;

	/*! \name Events */
	//@{

	virtual void eventKey(wchar_t unicode) = 0;

	virtual void eventKeyDown(int32_t keyCode) = 0;

	virtual void eventKeyUp(int32_t keyCode) = 0;

	virtual void eventMouseDown(const Vector2& position, int32_t button) = 0;

	virtual void eventMouseUp(const Vector2& position, int32_t button) = 0;

	virtual void eventMousePress(const Vector2& position, int32_t button) = 0;

	virtual void eventMouseRelease(const Vector2& position, int32_t button) = 0;

	virtual void eventMouseMove(const Vector2& position, int32_t button) = 0;

	virtual void eventMouseEnter(const Vector2& position, int32_t button) = 0;

	virtual void eventMouseLeave(const Vector2& position, int32_t button) = 0;

	virtual void eventMouseWheel(const Vector2& position, int32_t delta) = 0;

	virtual void eventViewResize(int32_t width, int32_t height) = 0;

	//@}
};

	}
}

#endif	// traktor_spark_IComponent_H
