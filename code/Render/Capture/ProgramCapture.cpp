#pragma optimize( "", off )

/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Misc/SafeDestroy.h"
#include "Render/Capture/CubeTextureCapture.h"
#include "Render/Capture/Error.h"
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
	T_CAPTURE_ASSERT (m_program, L"Program destroyed.");
	T_CAPTURE_ASSERT (handle, L"Null parameter handle.");

	if (!m_program)
		return;
	
	m_program->setFloatParameter(handle, param);

	std::map< handle_t, Parameter >::iterator it = m_shadow.find(handle);
	if (it != m_shadow.end())
	{
		T_CAPTURE_ASSERT (it->second.uniform, L"Incorrect parameter type, not a single uniform.");
		T_CAPTURE_ASSERT (it->second.uniform->getParameterType() == PtScalar, L"Incorrect parameter type, not scalar.");
		it->second.undefined = false;
	}
}

void ProgramCapture::setFloatArrayParameter(handle_t handle, const float* param, int length)
{
	T_CAPTURE_ASSERT (m_program, L"Program destroyed.");
	T_CAPTURE_ASSERT (handle, L"Null parameter handle.");
	T_CAPTURE_ASSERT (param, L"Null parameter array.");
	T_CAPTURE_ASSERT (length > 0, L"Array parameter zero length.");

	if (!m_program)
		return;

	m_program->setFloatArrayParameter(handle, param, length);

	std::map< handle_t, Parameter >::iterator it = m_shadow.find(handle);
	if (it != m_shadow.end())
	{
		T_CAPTURE_ASSERT (it->second.indexedUniform, L"Incorrect parameter type, not an indexed uniform.");
		T_CAPTURE_ASSERT (it->second.indexedUniform->getParameterType() == PtScalar, L"Incorrect parameter type, not scalar.");
		T_CAPTURE_ASSERT (it->second.indexedUniform->getLength() >= length, L"Trying to set too many elements of indexed uniform.");
		it->second.undefined = false;
	}
}

void ProgramCapture::setVectorParameter(handle_t handle, const Vector4& param)
{
	T_CAPTURE_ASSERT (m_program, L"Program destroyed.");
	T_CAPTURE_ASSERT (handle, L"Null parameter handle.");

	if (!m_program)
		return;

	m_program->setVectorParameter(handle, param);

	std::map< handle_t, Parameter >::iterator it = m_shadow.find(handle);
	if (it != m_shadow.end())
	{
		T_CAPTURE_ASSERT (it->second.uniform, L"Incorrect parameter type, not a single uniform.");
		T_CAPTURE_ASSERT (it->second.uniform->getParameterType() == PtVector, L"Incorrect parameter type, not vector.");
		it->second.undefined = false;
	}
}

void ProgramCapture::setVectorArrayParameter(handle_t handle, const Vector4* param, int length)
{
	T_CAPTURE_ASSERT (m_program, L"Program destroyed.");
	T_CAPTURE_ASSERT (handle, L"Null parameter handle.");
	T_CAPTURE_ASSERT (param, L"Null parameter array.");
	T_CAPTURE_ASSERT (length > 0, L"Array parameter zero length.");

	if (!m_program)
		return;

	m_program->setVectorArrayParameter(handle, param, length);

	std::map< handle_t, Parameter >::iterator it = m_shadow.find(handle);
	if (it != m_shadow.end())
	{
		T_CAPTURE_ASSERT (it->second.indexedUniform, L"Incorrect parameter type, not an indexed uniform.");
		T_CAPTURE_ASSERT (it->second.indexedUniform->getParameterType() == PtVector, L"Incorrect parameter type, not scalar.");
		T_CAPTURE_ASSERT (it->second.indexedUniform->getLength() >= length, L"Trying to set too many elements of indexed uniform.");
		it->second.undefined = false;
	}
}

void ProgramCapture::setMatrixParameter(handle_t handle, const Matrix44& param)
{
	T_CAPTURE_ASSERT (m_program, L"Program destroyed.");
	T_CAPTURE_ASSERT (handle, L"Null parameter handle.");

	if (!m_program)
		return;

	m_program->setMatrixParameter(handle, param);

	std::map< handle_t, Parameter >::iterator it = m_shadow.find(handle);
	if (it != m_shadow.end())
	{
		T_CAPTURE_ASSERT (it->second.uniform, L"Incorrect parameter type, not a single uniform.");
		T_CAPTURE_ASSERT (it->second.uniform->getParameterType() == PtMatrix, L"Incorrect parameter type, not matrix.");
		it->second.undefined = false;
	}
}

void ProgramCapture::setMatrixArrayParameter(handle_t handle, const Matrix44* param, int length)
{
	T_CAPTURE_ASSERT (m_program, L"Program destroyed.");
	T_CAPTURE_ASSERT (handle, L"Null parameter handle.");
	T_CAPTURE_ASSERT (param, L"Null parameter array.");
	T_CAPTURE_ASSERT (length > 0, L"Array parameter zero length.");

	if (!m_program)
		return;

	m_program->setMatrixArrayParameter(handle, param, length);

	std::map< handle_t, Parameter >::iterator it = m_shadow.find(handle);
	if (it != m_shadow.end())
	{
		T_CAPTURE_ASSERT (it->second.indexedUniform, L"Incorrect parameter type, not an indexed uniform.");
		T_CAPTURE_ASSERT (it->second.indexedUniform->getParameterType() == PtMatrix, L"Incorrect parameter type, not matrix.");
		T_CAPTURE_ASSERT (it->second.indexedUniform->getLength() <= length, L"Trying to set too many elements of indexed uniform.");
		it->second.undefined = false;
	}
}

void ProgramCapture::setTextureParameter(handle_t handle, ITexture* texture)
{
	T_CAPTURE_ASSERT (m_program, L"Program destroyed.");

	if (!m_program)
		return;

	if (texture)
	{
		if (CubeTextureCapture* cubeTexture = dynamic_type_cast< CubeTextureCapture* >(texture->resolve()))
		{
			T_CAPTURE_ASSERT (cubeTexture->getTexture(), L"Trying to set destroyed texture as shader parameter.");

			m_program->setTextureParameter(handle, cubeTexture->getTexture());

			std::map< handle_t, Parameter >::iterator it = m_shadow.find(handle);
			if (it != m_shadow.end())
			{
				T_CAPTURE_ASSERT (it->second.uniform, L"Incorrect parameter type, not a single uniform.");
				T_CAPTURE_ASSERT (it->second.uniform->getParameterType() == PtTextureCube, L"Incorrect parameter type, not texture CUBE.");
				it->second.undefined = false;
			}
		}
		else if (SimpleTextureCapture* simpleTexture = dynamic_type_cast< SimpleTextureCapture* >(texture->resolve()))
		{
			T_CAPTURE_ASSERT (simpleTexture->getTexture(), L"Trying to set destroyed texture as shader parameter.");

			m_program->setTextureParameter(handle, simpleTexture->getTexture());

			std::map< handle_t, Parameter >::iterator it = m_shadow.find(handle);
			if (it != m_shadow.end())
			{
				T_CAPTURE_ASSERT (it->second.uniform, L"Incorrect parameter type, not a single uniform.");
				T_CAPTURE_ASSERT (it->second.uniform->getParameterType() == PtTexture2D, L"Incorrect parameter type, not texture 2D.");
				it->second.undefined = false;
			}
		}
		else if (VolumeTextureCapture* volumeTexture = dynamic_type_cast< VolumeTextureCapture* >(texture->resolve()))
		{
			T_CAPTURE_ASSERT (volumeTexture->getTexture(), L"Trying to set destroyed texture as shader parameter.");

			m_program->setTextureParameter(handle, volumeTexture->getTexture());

			std::map< handle_t, Parameter >::iterator it = m_shadow.find(handle);
			if (it != m_shadow.end())
			{
				T_CAPTURE_ASSERT (it->second.uniform, L"Incorrect parameter type, not a single uniform.");
				T_CAPTURE_ASSERT (it->second.uniform->getParameterType() == PtTexture3D, L"Incorrect parameter type, not texture 3D.");
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
	T_CAPTURE_ASSERT (m_program, L"Program destroyed.");
	if (m_program)
		m_program->setStencilReference(stencilReference);
}

void ProgramCapture::verify()
{
	T_CAPTURE_ASSERT (m_program, L"Program destroyed.");

	for (std::map< handle_t, Parameter >::const_iterator i = m_shadow.begin(); i != m_shadow.end(); ++i)
	{
		T_CAPTURE_ASSERT (!i->second.undefined, L"Parameter \"" << i->second.getName() << L"\" not set, value undefined (" << m_tag << L").");
	}

	for (std::map< handle_t, Ref< ITexture > >::const_iterator i = m_boundTextures.begin(); i != m_boundTextures.end(); ++i)
	{
		if (!i->second)
			continue;

		if (CubeTextureCapture* cubeTexture = dynamic_type_cast< CubeTextureCapture* >(i->second->resolve()))
		{
			T_CAPTURE_ASSERT (cubeTexture->getTexture(), L"Trying to draw with destroyed texture (" << m_tag << L").");
		}
		else if (SimpleTextureCapture* simpleTexture = dynamic_type_cast< SimpleTextureCapture* >(i->second->resolve()))
		{
			T_CAPTURE_ASSERT (simpleTexture->getTexture(), L"Trying to draw with destroyed texture (" << m_tag << L").");
		}
		else if (VolumeTextureCapture* volumeTexture = dynamic_type_cast< VolumeTextureCapture* >(i->second->resolve()))
		{
			T_CAPTURE_ASSERT (volumeTexture->getTexture(), L"Trying to draw with destroyed texture (" << m_tag << L").");
		}
		else
			T_FATAL_ERROR;
	}
}

std::wstring ProgramCapture::Parameter::getName() const
{
	if (indexedUniform)
		return indexedUniform->getParameterName();
	else if (uniform)
		return uniform->getParameterName();
	else
		return L"<Null uniform>";
}

	}
}
