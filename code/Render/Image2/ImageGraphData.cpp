/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberAlignedVector.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Core/Serialization/MemberSmallMap.h"
#include "Render/Image2/ImageGraph.h"
#include "Render/Image2/ImageGraphData.h"
#include "Render/Image2/ImagePassData.h"
#include "Render/Image2/ImagePassStepData.h"
#include "Render/Image2/ImageStructBufferData.h"
#include "Render/Image2/ImageTargetSetData.h"
#include "Render/Image2/ImageTextureData.h"

namespace traktor::render
{
	namespace
	{

std::atomic< int32_t > s_instance = 0;

	}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ImageGraphData", 6, ImageGraphData, ISerializable)

Ref< ImageGraph > ImageGraphData::createInstance(resource::IResourceManager* resourceManager, IRenderSystem* renderSystem) const
{
	Ref< ImageGraph > instance = new ImageGraph(m_name);

	for (const auto& permutationBits : m_permutationBits)
	{
		instance->m_permutationBits.insert(
			getParameterHandle(permutationBits.first),
			permutationBits.second
		);
	}

	for (const auto& permutationData : m_permutations)
	{
		ImageGraph::Permutation& permutation = instance->m_permutations.push_back();

		permutation.mask = permutationData.mask;
		permutation.value = permutationData.value;

		for (auto sbufferData : permutationData.sbuffers)
		{
			Ref< const ImageStructBuffer > sbuffer = sbufferData->createInstance(s_instance);
			if (!sbuffer)
				return nullptr;
			permutation.sbuffers.push_back(sbuffer);
		}

		for (auto textureData : permutationData.textures)
		{
			Ref< const ImageTexture > texture = textureData->createInstance(resourceManager);
			if (!texture)
				return nullptr;
			permutation.textures.push_back(texture);
		}

		for (auto targetSetData : permutationData.targetSets)
		{
			Ref< const ImageTargetSet > targetSet = targetSetData->createInstance(s_instance);
			if (!targetSet)
				return nullptr;
			permutation.targetSets.push_back(targetSet);
		}

		for (auto passData : permutationData.passes)
		{
			Ref< const ImagePass > pass = passData->createInstance(resourceManager, renderSystem);
			if (!pass)
				return nullptr;
			permutation.passes.push_back(pass);
		}

		for (auto stepData : permutationData.steps)
		{
			Ref< const ImagePassStep > step = stepData->createInstance(resourceManager, renderSystem);
			if (!step)
				return nullptr;
			permutation.steps.push_back(step);
		}
	}

	++s_instance;
	return instance;
}

void ImageGraphData::serialize(ISerializer& s)
{
	T_FATAL_ASSERT(s.getVersion< ImageGraphData >() >= 6);

	s >> Member< std::wstring >(L"name", m_name);
	s >> MemberSmallMap< std::wstring, uint32_t >(L"permutationBits", m_permutationBits);
	s >> MemberAlignedVector< PermutationData, MemberComposite< PermutationData > >(L"permutations", m_permutations);
}

void ImageGraphData::PermutationData::serialize(ISerializer& s)
{
	s >> Member< uint32_t >(L"mask", mask);
	s >> Member< uint32_t >(L"value", value);
	s >> MemberRefArray< ImageStructBufferData >(L"sbuffers", sbuffers);
	s >> MemberRefArray< ImageTextureData >(L"textures", textures);
	s >> MemberRefArray< ImageTargetSetData >(L"targetSets", targetSets);
	s >> MemberRefArray< ImagePassData >(L"passes", passes);
	s >> MemberRefArray< ImagePassStepData >(L"steps", steps);
}

}
