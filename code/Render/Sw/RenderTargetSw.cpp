#include "Render/Sw/RenderTargetSw.h"
#include "Render/Sw/Samplers.h"
#include "Render/Sw/TypesSw.h"
#include "Render/Types.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

template <
	typename AddressU,
	typename AddressV
>
class RenderTargetSampler_TfR8G8B8A8 : public AbstractSampler
{
public:
	RenderTargetSampler_TfR8G8B8A8(const RenderTargetSw* renderTarget, const uint32_t* data)
	:	m_renderTarget(renderTarget)
	,	m_data(data)
	,	m_width(renderTarget->getWidth())
	,	m_height(renderTarget->getHeight())
	{
	}

	virtual Vector4 getSize() const
	{
		return Vector4(float(m_width), float(m_height), 0.0f, 0.0f);
	}

	virtual Vector4 get(const Vector4& texCoord) const
	{
		int x = int(m_width * texCoord.x());
		int y = int(m_height * texCoord.y());

		x = AddressU::eval(m_width, x);
		y = AddressV::eval(m_height, y);

		return fromARGB(m_data[x + y * m_width]);
	}

private:
	Ref< const RenderTargetSw > m_renderTarget;
	const uint32_t* m_data;
	int m_width;
	int m_height;
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderTargetSw", RenderTargetSw, ISimpleTexture)

RenderTargetSw::RenderTargetSw()
:	m_width(0)
,	m_height(0)
{
}

bool RenderTargetSw::create(const RenderTargetSetCreateDesc& setDesc, const RenderTargetCreateDesc& desc)
{
	if (desc.format >= TfDXT1)
		return false;
	if (setDesc.width <= 0 || setDesc.height <= 0)
		return false;

	m_width = setDesc.width;
	m_height = setDesc.height;

	m_color.reset(new uint32_t [setDesc.width * setDesc.height]);
	return true;
}

void RenderTargetSw::destroy()
{
	m_width =
	m_height = 0;
	m_color.release();
}

ITexture* RenderTargetSw::resolve()
{
	return this;
}

int RenderTargetSw::getWidth() const
{
	return m_width;
}

int RenderTargetSw::getHeight() const
{
	return m_height;
}

bool RenderTargetSw::lock(int level, Lock& lock)
{
	return false;
}

void RenderTargetSw::unlock(int level)
{
}

uint32_t* RenderTargetSw::getColorSurface()
{
	return m_color.ptr();
}

Ref< AbstractSampler > RenderTargetSw::createSampler() const
{
	return new RenderTargetSampler_TfR8G8B8A8< AddressWrap, AddressWrap >(this, m_color.c_ptr());
}

	}
}
