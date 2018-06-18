/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_IProgram_H
#define traktor_render_IProgram_H

#include "Core/Object.h"
#include "Render/Types.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{


class Vector4;
class Matrix44;

	namespace render
	{

class ITexture;

/*! \brief Shader program
 * \ingroup Render
 */
class T_DLLCLASS IProgram : public Object
{
	T_RTTI_CLASS;

public:
	virtual void destroy() = 0;

	virtual void setFloatParameter(handle_t handle, float param) = 0;

	virtual void setFloatArrayParameter(handle_t handle, const float* param, int length) = 0;

	virtual void setVectorParameter(handle_t handle, const Vector4& param) = 0;

	virtual void setVectorArrayParameter(handle_t handle, const Vector4* param, int length) = 0;

	virtual void setMatrixParameter(handle_t handle, const Matrix44& param) = 0;

	virtual void setMatrixArrayParameter(handle_t handle, const Matrix44* param, int length) = 0;

	virtual void setTextureParameter(handle_t handle, ITexture* texture) = 0;

	virtual void setStencilReference(uint32_t stencilReference) = 0;
};

	}
}

#endif	// traktor_render_IProgram_H
