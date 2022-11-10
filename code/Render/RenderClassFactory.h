/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Class/IRuntimeClassFactory.h"
#include "Core/Math/Matrix44.h"
#include "Render/Types.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::render
{

class IBufferView;
class ITexture;
class ProgramParameters;

class T_DLLCLASS RenderClassFactory : public IRuntimeClassFactory
{
	T_RTTI_CLASS;

public:
	virtual void createClasses(IRuntimeClassRegistrar* registrar) const override final;
};

class T_DLLCLASS BoxedProgramParameters : public Object
{
	T_RTTI_CLASS;

public:
	BoxedProgramParameters() = default;

	explicit BoxedProgramParameters(ProgramParameters* programParameters);

	void setProgramParameters(ProgramParameters* programParameters);

	void setFloatParameter(const handle_t handle, float param);

	void setVectorParameter(const handle_t handle, const Vector4& param);

	void setVectorArrayParameter(const handle_t handle, const AlignedVector< Vector4 >& param);

	void setMatrixParameter(handle_t handle, const Matrix44& param);

	void setMatrixArrayParameter(handle_t handle, const AlignedVector< Matrix44 >& param);

	void setTextureParameter(const handle_t handle, ITexture* texture);

	void setBufferViewParameter(const handle_t handle, const IBufferView* bufferView);

	void setStencilReference(uint32_t stencilReference);

	void* operator new (size_t size);

	void operator delete (void* ptr);

private:
	ProgramParameters* m_programParameters = nullptr;
};

}
