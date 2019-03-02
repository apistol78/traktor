#pragma once

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

	virtual void destroy() override final;

	virtual ITexture* resolve() override final;

	virtual int32_t getMips() const override final;

	virtual int32_t getSide() const override final;

	virtual bool lock(int32_t side, int32_t level, Lock& lock) override final;

	virtual void unlock(int32_t side, int32_t level) override final;

private:
	Ref< ContextPs4 > m_context;
};

	}
}
