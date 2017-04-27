/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_ICompressor_H
#define traktor_render_ICompressor_H

#include "Core/Object.h"
#include "Core/RefArray.h"
#include "Render/Types.h"

namespace traktor
{

class Writer;

	namespace drawing
	{

class Image;

	}

	namespace render
	{

class ICompressor : public Object
{
	T_RTTI_CLASS;

public:
	virtual bool compress(Writer& writer, const RefArray< drawing::Image >& mipImages, TextureFormat textureFormat, bool needAlpha, int32_t compressionQuality) const = 0;
};

	}
}

#endif	// traktor_render_ICompressor_H
