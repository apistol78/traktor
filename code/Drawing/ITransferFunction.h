/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_drawing_ITransferFunction_H
#define traktor_drawing_ITransferFunction_H

#include "Core/Object.h"
#include "Core/Math/Color4f.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_DRAWING_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace drawing
	{
	
class Image;

/*! \brief Image copy transfer function base class.
 * \ingroup Drawing
 */
class T_DLLCLASS ITransferFunction : public Object
{
	T_RTTI_CLASS;

protected:
	friend class Image;

	virtual void evaluate(const Color4f& in, Color4f& out) const = 0;
};
	
	}
}

#endif	// traktor_drawing_ITransferFunction_H
