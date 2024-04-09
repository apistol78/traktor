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

namespace traktor::render
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

	const auto it = m_shadow.find(handle);
	if (it != m_shadow.end())
	{
		T_CAPTURE_ASSERT(it->second.length <= 0, L"Incorrect parameter type, not a single uniform.");
		T_CAPTURE_ASSERT(it->second.type == ParameterType::Scalar, L"Incorrect parameter type, not scalar.");
		it->second.set = true;
	}
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

	const auto it = m_shadow.find(handle);
	if (it != m_shadow.end())
	{
		T_CAPTURE_ASSERT(it->second.length > 0, L"Incorrect parameter type, not an indexed uniform.");
		T_CAPTURE_ASSERT(it->second.length >= length, L"Trying to set too many elements of indexed uniform.");
		T_CAPTURE_ASSERT(it->second.type == ParameterType::Scalar, L"Incorrect parameter type, not scalar.");
		it->second.set = true;
	}
}

void ProgramVrfy::setVectorParameter(handle_t handle, const Vector4& param)
{
	T_CAPTURE_ASSERT(m_program, L"Program destroyed.");
	T_CAPTURE_ASSERT(handle, L"Null parameter handle.");

	if (!m_program)
		return;

	m_program->setVectorParameter(handle, param);

	const auto it = m_shadow.find(handle);
	if (it != m_shadow.end())
	{
		T_CAPTURE_ASSERT(it->second.length <= 0, L"Incorrect parameter type, not a single uniform.");
		T_CAPTURE_ASSERT(it->second.type == ParameterType::Vector, L"Incorrect parameter type, not vector.");
		it->second.set = true;
	}
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

	const auto it = m_shadow.find(handle);
	if (it != m_shadow.end())
	{
		T_CAPTURE_ASSERT(it->second.length > 0, L"Incorrect parameter type, not an indexed uniform.");
		T_CAPTURE_ASSERT(it->second.length >= length, L"Trying to set too many elements of indexed uniform.");
		T_CAPTURE_ASSERT(it->second.type == ParameterType::Vector, L"Incorrect parameter type, not vector.");
		it->second.set = true;
	}
}

void ProgramVrfy::setMatrixParameter(handle_t handle, const Matrix44& param)
{
	T_CAPTURE_ASSERT(m_program, L"Program destroyed.");
	T_CAPTURE_ASSERT(handle, L"Null parameter handle.");

	if (!m_program)
		return;

	m_program->setMatrixParameter(handle, param);

	const auto it = m_shadow.find(handle);
	if (it != m_shadow.end())
	{
		T_CAPTURE_ASSERT(it->second.length <= 0, L"Incorrect parameter type, not a single uniform.");
		T_CAPTURE_ASSERT(it->second.type == ParameterType::Matrix, L"Incorrect parameter type, not matrix.");
		it->second.set = true;
	}
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

	const auto it = m_shadow.find(handle);
	if (it != m_shadow.end())
	{
		T_CAPTURE_ASSERT(it->second.length > 0, L"Incorrect parameter type, not an indexed uniform.");
		T_CAPTURE_ASSERT(it->second.length >= length, L"Trying to set too many elements of indexed uniform.");
		T_CAPTURE_ASSERT(it->second.type == ParameterType::Matrix, L"Incorrect parameter type, not matrix.");
		it->second.set = true;
	}
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

	const auto it = m_shadow.find(handle);
	if (it != m_shadow.end())
	{
		T_CAPTURE_ASSERT(it->second.length <= 0, L"Incorrect parameter type, not a single uniform.");
		T_CAPTURE_ASSERT(
			(it->second.type == ParameterType::Texture2D) ||
			(it->second.type == ParameterType::TextureCube) ||
			(it->second.type == ParameterType::Texture3D),
			L"Incorrect parameter type, not texture."
		);
		it->second.set = true;
	}
}

void ProgramVrfy::setImageViewParameter(handle_t handle, ITexture* imageView, int mip)
{
	T_CAPTURE_ASSERT(m_program, L"Program destroyed.");

	if (!m_program)
		return;

	if (imageView)
	{
		if (TextureVrfy* textureVrfy = dynamic_type_cast< TextureVrfy* >(imageView->resolve()))
		{
			T_CAPTURE_ASSERT(textureVrfy->getTexture(), L"Trying to set destroyed texture as shader parameter.");
			T_CAPTURE_ASSERT(textureVrfy->shaderStorage(), L"Trying to set non-storage texture as image.");
		}
		else
			T_FATAL_ERROR;
	}
	else
		m_program->setImageViewParameter(handle, nullptr, 0);

	m_boundImages[handle] = { imageView, mip };

	const auto it = m_shadow.find(handle);
	if (it != m_shadow.end())
	{
		T_CAPTURE_ASSERT(it->second.length <= 0, L"Incorrect parameter type, not a single uniform.");
		T_CAPTURE_ASSERT(
			(it->second.type == ParameterType::Image2D) ||
			(it->second.type == ParameterType::ImageCube) ||
			(it->second.type == ParameterType::Image3D),
			L"Incorrect parameter type, not image."
		);
		it->second.set = true;
	}
}

void ProgramVrfy::setBufferViewParameter(handle_t handle, const IBufferView* bufferView)
{
	T_CAPTURE_ASSERT(m_program, L"Program destroyed.");

	if (m_program)
		m_program->setBufferViewParameter(handle, bufferView);

	const auto it = m_shadow.find(handle);
	if (it != m_shadow.end())
	{
		T_CAPTURE_ASSERT(it->second.length <= 0, L"Incorrect parameter type, not a single uniform.");
		T_CAPTURE_ASSERT(it->second.type == ParameterType::StructBuffer, L"Incorrect parameter type, not buffer.");
		it->second.set = true;
	}
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

	for (const auto& parameter : m_shadow)
	{
		T_CAPTURE_ASSERT(parameter.second.set, L"Parameter \"" << parameter.second.name << L"\" not set, value undefined (" << m_tag << L").");
	}

	for (auto i = m_boundTextures.begin(); i != m_boundTextures.end(); ++i)
	{
		if (!i->second)
			continue;

		if (TextureVrfy* textureVrfy = dynamic_type_cast< TextureVrfy* >(i->second->resolve()))
		{
			T_CAPTURE_ASSERT(textureVrfy->getTexture(), L"Trying to draw with destroyed texture " << getParameterName(i->first) << L" (" << m_tag << L").");
			m_program->setTextureParameter(i->first, textureVrfy->getTexture());
		}
	}

	for (auto i = m_boundImages.begin(); i != m_boundImages.end(); ++i)
	{
		const std::pair< Ref< ITexture >, int >& it = i->second;

		if (!it.first)
			continue;

		if (TextureVrfy* textureVrfy = dynamic_type_cast< TextureVrfy* >(it.first->resolve()))
		{
			T_CAPTURE_ASSERT(textureVrfy->getTexture(), L"Trying to draw with destroyed image " << getParameterName(i->first) << L" (" << m_tag << L").");
			m_program->setImageViewParameter(i->first, textureVrfy->getTexture(), it.second);
		}
	}
}

}
