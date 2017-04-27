/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_SHFunction_H
#define traktor_render_SHFunction_H

#include "Core/Object.h"
#include "Core/Math/Vector4.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

/*! \brief Spherical harmonics function.
 * \ingroup Render
 */
class T_DLLCLASS SHFunction : public Object
{
	T_RTTI_CLASS;

public:
	virtual float evaluate(float phi, float theta, const Vector4& unit) = 0;
};

	}
}

#endif	// traktor_render_SHFunction_H
