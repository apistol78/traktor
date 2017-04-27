/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_RenderTargetSetXbox360_H
#define traktor_render_RenderTargetSetXbox360_H

#include "Render/RenderTargetSet.h"

namespace traktor
{
	namespace render
	{

/*!
 * \ingroup DX9
 */
class RenderTargetSetXbox360 : public RenderTargetSet
{
	T_RTTI_CLASS;

public:
	virtual void destroy();

	virtual int getWidth() const;
	
	virtual int getHeight() const;
	
	virtual ISimpleTexture* getColorTexture(int index) const;

	virtual void swap(int index1, int index2);

	virtual void discard();

	virtual bool read(int index, void* buffer) const;
};

	}
}

#endif	// traktor_render_RenderTargetSetXbox360_H
