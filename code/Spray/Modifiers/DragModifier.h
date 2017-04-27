/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_spray_DragModifier_H
#define traktor_spray_DragModifier_H

#include "Spray/Modifier.h"

namespace traktor
{
	namespace spray
	{

/*! \brief Drag modifier.
 * \ingroup Spray
 */
class DragModifier : public Modifier
{
	T_RTTI_CLASS;

public:
	DragModifier(float linearDrag, float angularDrag);

#if defined(T_MODIFIER_USE_PS3_SPURS)
	virtual void update(SpursJobQueue* jobQueue, const Scalar& deltaTime, const Transform& transform, PointVector& points) const T_OVERRIDE T_FINAL;
#else
	virtual void update(const Scalar& deltaTime, const Transform& transform, PointVector& points, size_t first, size_t last) const T_OVERRIDE T_FINAL;
#endif

private:
	Scalar m_linearDrag;
	float m_angularDrag;
};

	}
}

#endif	// traktor_spray_DragModifier_H
