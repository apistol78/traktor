/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <string>
#include "Core/Containers/SmallMap.h"
#include "Render/IProgram.h"

namespace traktor::render
{

class ResourceTracker;

/*!
 * \ingroup Vrfy
 */
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

	virtual void setImageViewParameter(handle_t handle, ITexture* imageView, int mip) override final;

	virtual void setBufferViewParameter(handle_t handle, const IBufferView* bufferView) override final;

	virtual void setAccelerationStructureParameter(handle_t handle, const IAccelerationStructure* accelerationStructure) override final;

	virtual void setProgramDispatchTable(const IProgramDispatchTable* dispatchTable) override final;

	virtual void setStencilReference(uint32_t stencilReference) override final;

	void verify();

private:
	friend class RenderSystemVrfy;
	friend class RenderViewVrfy;

	struct Parameter
	{
		std::wstring name;
		ParameterType type;
		int32_t length;
		bool set;
	};

	Ref< ResourceTracker > m_resourceTracker;
	Ref< IProgram > m_program;
	std::wstring m_tag;
	SmallMap< handle_t, Parameter > m_shadow;
	SmallMap< handle_t, Ref< ITexture > > m_boundTextures;
	SmallMap< handle_t, std::pair< Ref< ITexture >, int > > m_boundImages;
};

}
