/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Misc/SafeDestroy.h"
#include "Render/Vrfy/Error.h"
#include "Render/Vrfy/ProgramVrfy.h"
#include "Render/Vrfy/ResourceTracker.h"
#include "Render/Vrfy/TextureVrfy.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ProgramVrfy", ProgramVrfy, IProgram)

ProgramVrfy::ProgramVrfy(ResourceTracker* resourceTracker, IProgram* program, const wchar_t* const tag)
:	m_resourceTracker(resourceTracker)
,	m_program(program)
,	m_tag(tag)
{
	m_resourceTracker->add(this);
}

ProgramVrfy::~ProgramVrfy()
{
	m_resourceTracker->remove(this);
}

void ProgramVrfy::destroy()
{
	T_CAPTURE_ASSERT (m_program, L"Program already destroyed.");
	safeDestroy(m_program);
}

void ProgramVrfy::setFloatParameter(handle_t handle, float param)
{
	T_CAPTURE_ASSERT(m_program, L"Program destroyed.");
	T_CAPTURE_ASSERT(handle, L"Null parameter handle.");

	if (!m_program)
		return;

	m_program->setFloatParameter(handle, param);

	// std::map< handle_t, Parameter >::iterator it = m_shadow.find(handle);
	// if (it != m_shadow.end())
	// {
	// 	T_CAPTURE_ASSERT(it->second.uniform, L"Incorrect parameter type, not a single uniform.");
	// 	T_CAPTURE_ASSERT(it->second.uniform->getParameterType() == ParameterType::Scalar, L"Incorrect parameter type, not scalar.");
	// 	it->second.undefined = false;
	// }
}

void ProgramVrfy::setFloatArrayParameter(handle_t handle, const float* param, int length)
{
	T_CAPTURE_ASSERT(m_program, L"Program destroyed.");
	T_CAPTURE_ASSERT(handle, L"Null parameter handle.");
	T_CAPTURE_ASSERT(param, L"Null parameter array.");
	T_CAPTURE_ASSERT(length > 0, L"Array parameter zero length.");

	if (!m_program)
		return;

	m_program->setFloatArrayParameter(handle, param, length);

	// std::map< handle_t, Parameter >::iterator it = m_shadow.find(handle);
	// if (it != m_shadow.end())
	// {
	// 	T_CAPTURE_ASSERT(it->second.indexedUniform, L"Incorrect parameter type, not an indexed uniform.");
	// 	T_CAPTURE_ASSERT(it->second.indexedUniform->getParameterType() == ParameterType::Scalar, L"Incorrect parameter type, not scalar.");
	// 	T_CAPTURE_ASSERT(it->second.indexedUniform->getLength() >= length, L"Trying to set too many elements of indexed uniform.");
	// 	it->second.undefined = false;
	// }
}

void ProgramVrfy::setVectorParameter(handle_t handle, const Vector4& param)
{
	T_CAPTURE_ASSERT(m_program, L"Program destroyed.");
	T_CAPTURE_ASSERT(handle, L"Null parameter handle.");

	if (!m_program)
		return;

	m_program->setVectorParameter(handle, param);

	// std::map< handle_t, Parameter >::iterator it = m_shadow.find(handle);
	// if (it != m_shadow.end())
	// {
	// 	T_CAPTURE_ASSERT(it->second.uniform, L"Incorrect parameter type, not a single uniform.");
	// 	T_CAPTURE_ASSERT(it->second.uniform->getParameterType() == ParameterType::Vector, L"Incorrect parameter type, not vector.");
	// 	it->second.undefined = false;
	// }
}

void ProgramVrfy::setVectorArrayParameter(handle_t handle, const Vector4* param, int length)
{
	T_CAPTURE_ASSERT(m_program, L"Program destroyed.");
	T_CAPTURE_ASSERT(handle, L"Null parameter handle.");
	T_CAPTURE_ASSERT(param, L"Null parameter array.");
	T_CAPTURE_ASSERT(length > 0, L"Array parameter zero length.");

	if (!m_program)
		return;

	m_program->setVectorArrayParameter(handle, param, length);

	// std::map< handle_t, Parameter >::iterator it = m_shadow.find(handle);
	// if (it != m_shadow.end())
	// {
	// 	T_CAPTURE_ASSERT(it->second.indexedUniform, L"Incorrect parameter type, not an indexed uniform.");
	// 	T_CAPTURE_ASSERT(it->second.indexedUniform->getParameterType() == ParameterType::Vector, L"Incorrect parameter type, not scalar.");
	// 	T_CAPTURE_ASSERT(it->second.indexedUniform->getLength() >= length, L"Trying to set too many elements of indexed uniform.");
	// 	it->second.undefined = false;
	// }
}

void ProgramVrfy::setMatrixParameter(handle_t handle, const Matrix44& param)
{
	T_CAPTURE_ASSERT(m_program, L"Program destroyed.");
	T_CAPTURE_ASSERT(handle, L"Null parameter handle.");

	if (!m_program)
		return;

	m_program->setMatrixParameter(handle, param);

	// std::map< handle_t, Parameter >::iterator it = m_shadow.find(handle);
	// if (it != m_shadow.end())
	// {
	// 	T_CAPTURE_ASSERT(it->second.uniform, L"Incorrect parameter type, not a single uniform.");
	// 	T_CAPTURE_ASSERT(it->second.uniform->getParameterType() == ParameterType::Matrix, L"Incorrect parameter type, not matrix.");
	// 	it->second.undefined = false;
	// }
}

void ProgramVrfy::setMatrixArrayParameter(handle_t handle, const Matrix44* param, int length)
{
	T_CAPTURE_ASSERT(m_program, L"Program destroyed.");
	T_CAPTURE_ASSERT(handle, L"Null parameter handle.");
	T_CAPTURE_ASSERT(param, L"Null parameter array.");
	T_CAPTURE_ASSERT(length > 0, L"Array parameter zero length.");

	if (!m_program)
		return;

	m_program->setMatrixArrayParameter(handle, param, length);

	// std::map< handle_t, Parameter >::iterator it = m_shadow.find(handle);
	// if (it != m_shadow.end())
	// {
	// 	T_CAPTURE_ASSERT(it->second.indexedUniform, L"Incorrect parameter type, not an indexed uniform.");
	// 	T_CAPTURE_ASSERT(it->second.indexedUniform->getParameterType() == ParameterType::Matrix, L"Incorrect parameter type, not matrix.");
	// 	T_CAPTURE_ASSERT(it->second.indexedUniform->getLength() <= length, L"Trying to set too many elements of indexed uniform.");
	// 	it->second.undefined = false;
	// }
}

void ProgramVrfy::setTextureParameter(handle_t handle, ITexture* texture)
{
	T_CAPTURE_ASSERT(m_program, L"Program destroyed.");

	if (!m_program)
		return;

	if (texture)
	{
		if (TextureVrfy* textureVrfy = dynamic_type_cast< TextureVrfy* >(texture->resolve()))
		{
			T_CAPTURE_ASSERT(textureVrfy->getTexture(), L"Trying to set destroyed texture as shader parameter.");
		}
		else
			T_FATAL_ERROR;
	}
	else
		m_program->setTextureParameter(handle, nullptr);

	m_boundTextures[handle] = texture;
}

void ProgramVrfy::setImageViewParameter(handle_t handle, ITexture* imageView)
{
	T_CAPTURE_ASSERT(m_program, L"Program destroyed.");
	m_program->setImageViewParameter(handle, imageView);
}

void ProgramVrfy::setBufferViewParameter(handle_t handle, const IBufferView* bufferView)
{
	T_CAPTURE_ASSERT(m_program, L"Program destroyed.");
	
	//if (!m_program)
	//	return;

	//if (StructBufferVrfy* sbufferVrfy = dynamic_type_cast< StructBufferVrfy* >(structBuffer))
	//{
	//	T_CAPTURE_ASSERT(sbufferVrfy->getStructBuffer(), L"Trying to set destroyed sbuffer as shader parameter.");
	//	m_program->setStructBufferParameter(handle, sbufferVrfy->getStructBuffer());
	//}
	//else
	//	T_FATAL_ERROR;

	if (m_program)
		m_program->setBufferViewParameter(handle, bufferView);
}

void ProgramVrfy::setStencilReference(uint32_t stencilReference)
{
	T_CAPTURE_ASSERT(m_program, L"Program destroyed.");
	if (m_program)
		m_program->setStencilReference(stencilReference);
}

void ProgramVrfy::verify()
{
	T_CAPTURE_ASSERT(m_program, L"Program destroyed.");

	// for (std::map< handle_t, Parameter >::const_iterator i = m_shadow.begin(); i != m_shadow.end(); ++i)
	// {
	// 	T_CAPTURE_ASSERT(!i->second.undefined, L"Parameter \"" << i->second.getName() << L"\" not set, value undefined (" << m_tag << L").");
	// }

	for (auto i = m_boundTextures.begin(); i != m_boundTextures.end(); ++i)
	{
		if (!i->second)
			continue;

		if (TextureVrfy* textureVrfy = dynamic_type_cast< TextureVrfy* >(i->second->resolve()))
		{
			T_CAPTURE_ASSERT(textureVrfy->getTexture(), L"Trying to draw with destroyed texture (" << m_tag << L").");
			m_program->setTextureParameter(i->first, textureVrfy->getTexture());
		}
	}
}

// std::wstring ProgramVrfy::Parameter::getName() const
// {
// 	if (indexedUniform)
// 		return indexedUniform->getParameterName();
// 	else if (uniform)
// 		return uniform->getParameterName();
// 	else
// 		return L"<Null uniform>";
// }

	}
}
