/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_terrain_ImageFallOff_H
#define traktor_terrain_ImageFallOff_H

#include "Terrain/Editor/IFallOff.h"

namespace traktor
{
	namespace drawing
	{

class Image;

	}

	namespace terrain
	{

class ImageFallOff : public IFallOff
{
	T_RTTI_CLASS;

public:
	ImageFallOff(const drawing::Image* image);

	virtual float evaluate(float x, float y) const T_OVERRIDE T_FINAL;

private:
	Ref< const drawing::Image > m_image;
};

	}
}

#endif	// traktor_terrain_ImageFallOff_H
