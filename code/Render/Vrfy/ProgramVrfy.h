#pragma once

#include <string>
#include "Core/Containers/SmallMap.h"
#include "Render/IProgram.h"

namespace traktor
{
	namespace render
	{

class ResourceTracker;

class ProgramVrfy : public IProgram
{
	T_RTTI_CLASS;

public:
	explicit ProgramVrfy(ResourceTracker* resourceTracker, IProgram* program, const wchar_t* const tag);

	virtual ~ProgramVrfy();

	virtual void destroy() override final;

	virtual void setFloatParameter(handle_t handle, float param) override final;

	virtual void setFloatArrayParameter(handle_t handle, const float* param, int length) override final;

	virtual void setVectorParameter(handle_t handle, const Vector4& param) override final;

	virtual void setVectorArrayParameter(handle_t handle, const Vector4* param, int length) override final;

	virtual void setMatrixParameter(handle_t handle, const Matrix44& param) override final;

	virtual void setMatrixArrayParameter(handle_t handle, const Matrix44* param, int length) override final;

	virtual void setTextureParameter(handle_t handle, ITexture* texture) override final;

	virtual void setImageViewParameter(handle_t handle, ITexture* imageView) override final;

	virtual void setBufferViewParameter(handle_t handle, const IBufferView* bufferView) override final;

	virtual void setStencilReference(uint32_t stencilReference) override final;

	void verify();

private:
	friend class RenderSystemVrfy;
	friend class RenderViewVrfy;

	struct Parameter
	{
		// Ref< const Uniform > uniform;
		// Ref< const IndexedUniform > indexedUniform;
		bool undefined;

		// std::wstring getName() const;
	};

	Ref< ResourceTracker > m_resourceTracker;
	Ref< IProgram > m_program;
	std::wstring m_tag;
	SmallMap< handle_t, Parameter > m_shadow;
	SmallMap< handle_t, Ref< ITexture > > m_boundTextures;
};

	}
}

