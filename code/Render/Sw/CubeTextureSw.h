/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_CubeTextureSw_H
#define traktor_render_CubeTextureSw_H

#include "Render/ICubeTexture.h"
#include "Render/Types.h"

namespace traktor
{
	namespace render
	{
		
/*!
 * \ingroup SW
 */
class CubeTextureSw : public ICubeTexture
{
	T_RTTI_CLASS;

public:
	CubeTextureSw();

	virtual ~CubeTextureSw();
	
	bool create(const CubeTextureCreateDesc& desc);

	virtual void destroy();

	virtual ITexture* resolve();

	virtual int getWidth() const;
	
	virtual int getHeight() const;
	
	virtual int getDepth() const;

	virtual bool lock(int side, int level, Lock& lock);

	virtual void unlock(int side, int level);

	inline const uint32_t* getData() const { return m_data; }

private:
	int m_side;
	TextureFormat m_format;
	uint32_t* m_data;
};
		
	}
}

#endif	// traktor_render_CubeTextureSw_H
