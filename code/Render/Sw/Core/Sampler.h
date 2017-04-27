/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_Sampler_H
#define traktor_render_Sampler_H

#include "Core/Object.h"
#include "Core/Math/Vector4.h"

namespace traktor
{
	namespace render
	{

/*! \brief Texture sampler interface.
 * \ingroup SW
 */
class AbstractSampler : public Object
{
public:
	virtual Vector4 getSize() const = 0;

	virtual Vector4 get(const Vector4& texCoord) const = 0;
};

	}
}

#endif	// traktor_render_Sampler_H
