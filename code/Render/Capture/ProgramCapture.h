/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_ProgramCapture_H
#define traktor_render_ProgramCapture_H

#include <map>
#include "Render/IProgram.h"

namespace traktor
{
	namespace render
	{

class ProgramCapture : public IProgram
{
	T_RTTI_CLASS;

public:
	ProgramCapture(IProgram* program, const wchar_t* const tag);

	virtual void destroy() override final;

	virtual void setFloatParameter(handle_t handle, float param) override final;

	virtual void setFloatArrayParameter(handle_t handle, const float* param, int length) override final;

	virtual void setVectorParameter(handle_t handle, const Vector4& param) override final;

	virtual void setVectorArrayParameter(handle_t handle, const Vector4* param, int length) override final;

	virtual void setMatrixParameter(handle_t handle, const Matrix44& param) override final;

	virtual void setMatrixArrayParameter(handle_t handle, const Matrix44* param, int length) override final;

	virtual void setTextureParameter(handle_t handle, ITexture* texture) override final;

	virtual void setStencilReference(uint32_t stencilReference) override final;

	void verify();

private:
	friend class RenderSystemCapture;
	friend class RenderViewCapture;

	struct Parameter
	{
		// Ref< const Uniform > uniform;
		// Ref< const IndexedUniform > indexedUniform;
		bool undefined;

		// std::wstring getName() const;
	};

	Ref< IProgram > m_program;
	std::wstring m_tag;
	std::map< handle_t, Parameter > m_shadow;
	std::map< handle_t, Ref< ITexture > > m_boundTextures;
};

	}
}

#endif	// traktor_render_ProgramCapture_H
