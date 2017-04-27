/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_SimpleTexturePs4_H
#define traktor_render_SimpleTexturePs4_H

#include "Render/ISimpleTexture.h"

namespace traktor
{
	namespace render
	{

class ContextPs4;
struct SimpleTextureCreateDesc;
		
/*!
 * \ingroup GNM
 */
class SimpleTexturePs4 : public ISimpleTexture
{
	T_RTTI_CLASS;

public:
	SimpleTexturePs4(ContextPs4* context);

	virtual ~SimpleTexturePs4();

	bool create(const SimpleTextureCreateDesc& desc);

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual ITexture* resolve() T_OVERRIDE T_FINAL;

	virtual int getWidth() const T_OVERRIDE T_FINAL;
	
	virtual int getHeight() const T_OVERRIDE T_FINAL;
	
	virtual bool lock(int level, Lock& lock) T_OVERRIDE T_FINAL;

	virtual void unlock(int level) T_OVERRIDE T_FINAL;

private:
	Ref< ContextPs4 > m_context;
};
		
	}
}

#endif	// traktor_render_SimpleTexturePs4_H
