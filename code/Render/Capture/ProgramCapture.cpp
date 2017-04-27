/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Misc/SafeDestroy.h"
#include "Render/Capture/CubeTextureCapture.h"
#include "Render/Capture/ProgramCapture.h"
#include "Render/Capture/SimpleTextureCapture.h"
#include "Render/Capture/VolumeTextureCapture.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ProgramCapture", ProgramCapture, IProgram)

ProgramCapture::ProgramCapture(IProgram* program, const wchar_t* const tag)
:	m_program(program)
,	m_tag(tag)
{
}

void ProgramCapture::destroy()
{
	safeDestroy(m_program);
}

void ProgramCapture::setFloatParameter(handle_t handle, float param)
{
	T_FATAL_ASSERT_M (m_program, L"Render error: Program destroyed.");
	T_FATAL_ASSERT_M (handle, L"Render error: Null parameter handle.");
	m_program->setFloatParameter(handle, param);
}

void ProgramCapture::setFloatArrayParameter(handle_t handle, const float* param, int length)
{
	T_FATAL_ASSERT_M (m_program, L"Render error: Program destroyed.");
	T_FATAL_ASSERT_M (handle, L"Render error: Null parameter handle.");
	T_FATAL_ASSERT_M (param, L"Render error: Null parameter array.");
	T_FATAL_ASSERT_M (length > 0, L"Render error: Array parameter zero length.");
	m_program->setFloatArrayParameter(handle, param, length);
}

void ProgramCapture::setVectorParameter(handle_t handle, const Vector4& param)
{
	T_FATAL_ASSERT_M (m_program, L"Render error: Program destroyed.");
	T_FATAL_ASSERT_M (handle, L"Render error: Null parameter handle.");
	m_program->setVectorParameter(handle, param);
}

void ProgramCapture::setVectorArrayParameter(handle_t handle, const Vector4* param, int length)
{
	T_FATAL_ASSERT_M (m_program, L"Render error: Program destroyed.");
	T_FATAL_ASSERT_M (handle, L"Render error: Null parameter handle.");
	T_FATAL_ASSERT_M (param, L"Render error: Null parameter array.");
	T_FATAL_ASSERT_M (length > 0, L"Render error: Array parameter zero length.");
	m_program->setVectorArrayParameter(handle, param, length);
}

void ProgramCapture::setMatrixParameter(handle_t handle, const Matrix44& param)
{
	T_FATAL_ASSERT_M (m_program, L"Render error: Program destroyed.");
	T_FATAL_ASSERT_M (handle, L"Render error: Null parameter handle.");
	m_program->setMatrixParameter(handle, param);
}

void ProgramCapture::setMatrixArrayParameter(handle_t handle, const Matrix44* param, int length)
{
	T_FATAL_ASSERT_M (m_program, L"Render error: Program destroyed.");
	T_FATAL_ASSERT_M (handle, L"Render error: Null parameter handle.");
	T_FATAL_ASSERT_M (param, L"Render error: Null parameter array.");
	T_FATAL_ASSERT_M (length > 0, L"Render error: Array parameter zero length.");
	m_program->setMatrixArrayParameter(handle, param, length);
}

void ProgramCapture::setTextureParameter(handle_t handle, ITexture* texture)
{
	T_FATAL_ASSERT_M (m_program, L"Render error: Program destroyed.");

	if (texture)
	{
		if (CubeTextureCapture* cubeTexture = dynamic_type_cast< CubeTextureCapture* >(texture->resolve()))
		{
			T_FATAL_ASSERT_M (cubeTexture->getTexture(), L"Render error: Trying to set destroyed texture as shader parameter.");
			m_program->setTextureParameter(handle, cubeTexture->getTexture());
		}
		else if (SimpleTextureCapture* simpleTexture = dynamic_type_cast< SimpleTextureCapture* >(texture->resolve()))
		{
			T_FATAL_ASSERT_M (simpleTexture->getTexture(), L"Render error: Trying to set destroyed texture as shader parameter.");
			m_program->setTextureParameter(handle, simpleTexture->getTexture());
		}
		else if (VolumeTextureCapture* volumeTexture = dynamic_type_cast< VolumeTextureCapture* >(texture->resolve()))
		{
			T_FATAL_ASSERT_M (volumeTexture->getTexture(), L"Render error: Trying to set destroyed texture as shader parameter.");
			m_program->setTextureParameter(handle, volumeTexture->getTexture());
		}
		else
			T_FATAL_ERROR;
	}
	else
		m_program->setTextureParameter(handle, 0);

	m_boundTextures[handle] = texture;
}

void ProgramCapture::setStencilReference(uint32_t stencilReference)
{
	T_FATAL_ASSERT_M (m_program, L"Render error: Program destroyed.");
	m_program->setStencilReference(stencilReference);
}

void ProgramCapture::verify()
{
	for (std::map< handle_t, Ref< ITexture > >::const_iterator i = m_boundTextures.begin(); i != m_boundTextures.end(); ++i)
	{
		if (!i->second)
			continue;

		if (CubeTextureCapture* cubeTexture = dynamic_type_cast< CubeTextureCapture* >(i->second->resolve()))
		{
			T_FATAL_ASSERT_M (cubeTexture->getTexture(), L"Render error: Trying to draw with destroyed texture.");
		}
		else if (SimpleTextureCapture* simpleTexture = dynamic_type_cast< SimpleTextureCapture* >(i->second->resolve()))
		{
			T_FATAL_ASSERT_M (simpleTexture->getTexture(), L"Render error: Trying to draw with destroyed texture.");
		}
		else if (VolumeTextureCapture* volumeTexture = dynamic_type_cast< VolumeTextureCapture* >(i->second->resolve()))
		{
			T_FATAL_ASSERT_M (volumeTexture->getTexture(), L"Render error: Trying to draw with destroyed texture.");
		}
		else
			T_FATAL_ERROR;
	}
}

	}
}
