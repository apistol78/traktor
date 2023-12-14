/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Core/Serialization/MemberStaticArray.h"
#include "Render/Image2/ImagePass.h"
#include "Render/Image2/ImagePassData.h"
#include "Render/Image2/ImagePassStepData.h"

namespace traktor::render
{
	namespace
	{

class MemberClear : public MemberComplex
{
public:
	MemberClear(const wchar_t* const name, Clear& ref)
	:	MemberComplex(name, true)
	,	m_ref(ref)
	{
	}

	virtual void serialize(ISerializer& s) const override final
	{
		s >> Member< uint32_t >(L"mask", m_ref.mask);
		s >> MemberStaticArray< Color4f, sizeof_array(m_ref.colors) >(L"colors", m_ref.colors);
		s >> Member< float >(L"depth", m_ref.depth);
		s >> Member< int32_t >(L"stencil", m_ref.stencil);
	}

private:
	Clear& m_ref;
};

	   }

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ImagePassData", 1, ImagePassData, ISerializable)

Ref< const ImagePass > ImagePassData::createInstance(resource::IResourceManager* resourceManager, IRenderSystem* renderSystem) const
{
	Ref< ImagePass > instance = new ImagePass();

	instance->m_name = m_name;
	instance->m_output = { m_outputTargetSet, m_outputSBuffer };
	instance->m_clear = m_clear;

	for (auto stepData : m_steps)
	{
		Ref< const ImagePassStep > step = stepData->createInstance(resourceManager, renderSystem);
		if (!step)
			return nullptr;
		instance->m_steps.push_back(step);
	}

	return instance;
}

void ImagePassData::serialize(ISerializer& s)
{
	T_FATAL_ASSERT(s.getVersion< ImagePassData >() >= 1);
	s >> Member< std::wstring >(L"name", m_name);
	s >> Member< int32_t >(L"outputTargetSet", m_outputTargetSet);
	s >> Member< int32_t >(L"outputSBuffer", m_outputSBuffer);
	s >> MemberClear(L"clear", m_clear);
	s >> MemberRefArray< ImagePassStepData >(L"steps", m_steps);
}

}
