/*
 * TRAKTOR
 * Copyright (c) 2022-2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberEnum.h"
#include "Core/Serialization/MemberStaticArray.h"
#include "Render/IRenderSystem.h"
#include "Render/Shader.h"
#include "Render/Image2/Compute.h"
#include "Render/Image2/ComputeData.h"
#include "Resource/IResourceManager.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ComputeData", 0, ComputeData, ImagePassStepData)

Ref< const ImagePassStep > ComputeData::createInstance(resource::IResourceManager* resourceManager, IRenderSystem* renderSystem) const
{
	Ref< Compute > instance = new Compute();
	instance->m_workSize = m_workSize;
	instance->m_manualWorkSize[0] = m_manualWorkSize[0];
	instance->m_manualWorkSize[1] = m_manualWorkSize[1];
	instance->m_manualWorkSize[2] = m_manualWorkSize[2];

	// Bind shader.
	if (!resourceManager->bind(m_shader, instance->m_shader))
		return nullptr;

	// Get handles of sources.
	setSourceHandles(instance);

	return instance; 
}

void ComputeData::serialize(ISerializer& s)
{
	ImagePassStepData::serialize(s);

	s >> MemberEnumByValue< WorkSize >(L"workSize", m_workSize);
	s >> MemberStaticArray< int32_t, 3 >(L"manualWorkSize", m_manualWorkSize);
}

}
