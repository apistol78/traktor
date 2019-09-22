#pragma once

#include "Core/Ref.h"
#include "Render/ICubeTexture.h"

namespace traktor
{
	namespace render
	{

/*!
 * \ingroup Capture
 */
class CubeTextureCapture : public ICubeTexture
{
	T_RTTI_CLASS;

public:
	CubeTextureCapture(ICubeTexture* texture);

	virtual void destroy() override final;

	virtual ITexture* resolve() override final;

	virtual int32_t getMips() const override final;

	virtual int32_t getSide() const override final;

	virtual bool lock(int32_t side, int32_t level, Lock& lock) override final;

	virtual void unlock(int32_t side, int32_t level) override final;

	virtual bool copy(int32_t side, int32_t level, const ISimpleTexture* sourceTexture) override final;

	ICubeTexture* getTexture() const { return m_texture; }

private:
	Ref< ICubeTexture > m_texture;
	int32_t m_locked[2];
};

	}
}
