#pragma optimize( "", off )

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
#include "Render/Shader/Nodes.h"

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

	std::map< handle_t, Parameter >::iterator it = m_shadow.find(handle);
	if (it != m_shadow.end())
	{
		T_FATAL_ASSERT_M (it->second.uniform, L"Render error: Incorrect parameter type, not a single uniform.");
		T_FATAL_ASSERT_M (it->second.uniform->getParameterType() == PtScalar, L"Render error: Incorrect parameter type, not scalar.");
		it->second.undefined = false;
	}
}

void ProgramCapture::setFloatArrayParameter(handle_t handle, const float* param, int length)
{
	T_FATAL_ASSERT_M (m_program, L"Render error: Program destroyed.");
	T_FATAL_ASSERT_M (handle, L"Render error: Null parameter handle.");
	T_FATAL_ASSERT_M (param, L"Render error: Null parameter array.");
	T_FATAL_ASSERT_M (length > 0, L"Render error: Array parameter zero length.");

	m_program->setFloatArrayParameter(handle, param, length);

	std::map< handle_t, Parameter >::iterator it = m_shadow.find(handle);
	if (it != m_shadow.end())
	{
		T_FATAL_ASSERT_M (it->second.indexedUniform, L"Render error: Incorrect parameter type, not an indexed uniform.");
		T_FATAL_ASSERT_M (it->second.indexedUniform->getParameterType() == PtScalar, L"Render error: Incorrect parameter type, not scalar.");
		T_FATAL_ASSERT_M (it->second.indexedUniform->getLength() >= length, L"Render error: Trying to set too many elements of indexed uniform.");
		it->second.undefined = false;
	}
}

void ProgramCapture::setVectorParameter(handle_t handle, const Vector4& param)
{
	T_FATAL_ASSERT_M (m_program, L"Render error: Program destroyed.");
	T_FATAL_ASSERT_M (handle, L"Render error: Null parameter handle.");

	m_program->setVectorParameter(handle, param);

	std::map< handle_t, Parameter >::iterator it = m_shadow.find(handle);
	if (it != m_shadow.end())
	{
		T_FATAL_ASSERT_M (it->second.uniform, L"Render error: Incorrect parameter type, not a single uniform.");
		T_FATAL_ASSERT_M (it->second.uniform->getParameterType() == PtVector, L"Render error: Incorrect parameter type, not vector.");
		it->second.undefined = false;
	}
}

void ProgramCapture::setVectorArrayParameter(handle_t handle, const Vector4* param, int length)
{
	T_FATAL_ASSERT_M (m_program, L"Render error: Program destroyed.");
	T_FATAL_ASSERT_M (handle, L"Render error: Null parameter handle.");
	T_FATAL_ASSERT_M (param, L"Render error: Null parameter array.");
	T_FATAL_ASSERT_M (length > 0, L"Render error: Array parameter zero length.");

	m_program->setVectorArrayParameter(handle, param, length);

	std::map< handle_t, Parameter >::iterator it = m_shadow.find(handle);
	if (it != m_shadow.end())
	{
		T_FATAL_ASSERT_M (it->second.indexedUniform, L"Render error: Incorrect parameter type, not an indexed uniform.");
		T_FATAL_ASSERT_M (it->second.indexedUniform->getParameterType() == PtVector, L"Render error: Incorrect parameter type, not scalar.");
		T_FATAL_ASSERT_M (it->second.indexedUniform->getLength() >= length, L"Render error: Trying to set too many elements of indexed uniform.");
		it->second.undefined = false;
	}
}

void ProgramCapture::setMatrixParameter(handle_t handle, const Matrix44& param)
{
	T_FATAL_ASSERT_M (m_program, L"Render error: Program destroyed.");
	T_FATAL_ASSERT_M (handle, L"Render error: Null parameter handle.");

	m_program->setMatrixParameter(handle, param);

	std::map< handle_t, Parameter >::iterator it = m_shadow.find(handle);
	if (it != m_shadow.end())
	{
		T_FATAL_ASSERT_M (it->second.uniform, L"Render error: Incorrect parameter type, not a single uniform.");
		T_FATAL_ASSERT_M (it->second.uniform->getParameterType() == PtMatrix, L"Render error: Incorrect parameter type, not matrix.");
		it->second.undefined = false;
	}
}

void ProgramCapture::setMatrixArrayParameter(handle_t handle, const Matrix44* param, int length)
{
	T_FATAL_ASSERT_M (m_program, L"Render error: Program destroyed.");
	T_FATAL_ASSERT_M (handle, L"Render error: Null parameter handle.");
	T_FATAL_ASSERT_M (param, L"Render error: Null parameter array.");
	T_FATAL_ASSERT_M (length > 0, L"Render error: Array parameter zero length.");

	m_program->setMatrixArrayParameter(handle, param, length);

	std::map< handle_t, Parameter >::iterator it = m_shadow.find(handle);
	if (it != m_shadow.end())
	{
		T_FATAL_ASSERT_M (it->second.indexedUniform, L"Render error: Incorrect parameter type, not an indexed uniform.");
		T_FATAL_ASSERT_M (it->second.indexedUniform->getParameterType() == PtMatrix, L"Render error: Incorrect parameter type, not matrix.");
		T_FATAL_ASSERT_M (it->second.indexedUniform->getLength() <= length, L"Render error: Trying to set too many elements of indexed uniform.");
		it->second.undefined = false;
	}
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

			std::map< handle_t, Parameter >::iterator it = m_shadow.find(handle);
			if (it != m_shadow.end())
			{
				T_FATAL_ASSERT_M (it->second.uniform, L"Render error: Incorrect parameter type, not a single uniform.");
				T_FATAL_ASSERT_M (it->second.uniform->getParameterType() == PtTextureCube, L"Render error: Incorrect parameter type, not texture CUBE.");
				it->second.undefined = false;
			}
		}
		else if (SimpleTextureCapture* simpleTexture = dynamic_type_cast< SimpleTextureCapture* >(texture->resolve()))
		{
			T_FATAL_ASSERT_M (simpleTexture->getTexture(), L"Render error: Trying to set destroyed texture as shader parameter.");

			m_program->setTextureParameter(handle, simpleTexture->getTexture());

			std::map< handle_t, Parameter >::iterator it = m_shadow.find(handle);
			if (it != m_shadow.end())
			{
				T_FATAL_ASSERT_M (it->second.uniform, L"Render error: Incorrect parameter type, not a single uniform.");
				T_FATAL_ASSERT_M (it->second.uniform->getParameterType() == PtTexture2D, L"Render error: Incorrect parameter type, not texture 2D.");
				it->second.undefined = false;
			}
		}
		else if (VolumeTextureCapture* volumeTexture = dynamic_type_cast< VolumeTextureCapture* >(texture->resolve()))
		{
			T_FATAL_ASSERT_M (volumeTexture->getTexture(), L"Render error: Trying to set destroyed texture as shader parameter.");

			m_program->setTextureParameter(handle, volumeTexture->getTexture());

			std::map< handle_t, Parameter >::iterator it = m_shadow.find(handle);
			if (it != m_shadow.end())
			{
				T_FATAL_ASSERT_M (it->second.uniform, L"Render error: Incorrect parameter type, not a single uniform.");
				T_FATAL_ASSERT_M (it->second.uniform->getParameterType() == PtTexture3D, L"Render error: Incorrect parameter type, not texture 3D.");
				it->second.undefined = false;
			}
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
	for (std::map< handle_t, Parameter >::const_iterator i = m_shadow.begin(); i != m_shadow.end(); ++i)
	{
		T_FATAL_ASSERT_M (!i->second.undefined, L"Render error: Parameter not set, value undefined.");
	}

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
