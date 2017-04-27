/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_CubeTexturePs4_H
#define traktor_render_CubeTexturePs4_H

#include "Render/ICubeTexture.h"

namespace traktor
{
	namespace render
	{

class ContextPs4;
struct CubeTextureCreateDesc;

/*!
 * \ingroup GNM
 */
class CubeTexturePs4 : public ICubeTexture
{
	T_RTTI_CLASS;

public:
	CubeTexturePs4(ContextPs4* context);

	virtual ~CubeTexturePs4();

	bool create(const CubeTextureCreateDesc& desc);

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual ITexture* resolve() T_OVERRIDE T_FINAL;

	virtual int getWidth() const T_OVERRIDE T_FINAL;
	
	virtual int getHeight() const T_OVERRIDE T_FINAL;
	
	virtual int getDepth() const T_OVERRIDE T_FINAL;

	virtual bool lock(int side, int level, Lock& lock) T_OVERRIDE T_FINAL;

	virtual void unlock(int side, int level) T_OVERRIDE T_FINAL;

private:
	Ref< ContextPs4 > m_context;
};
		
	}
}

#endif	// traktor_render_CubeTexturePs4_H
