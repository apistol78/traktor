/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <map>
#include "Animation/Joint.h"
#include "Animation/Skeleton.h"
#include "Animation/Editor/SkeletonFormatLws.h"
#include "Animation/Editor/LwsParser/LwsDocument.h"
#include "Animation/Editor/LwsParser/LwsGroup.h"
#include "Animation/Editor/LwsParser/LwsValue.h"
#include "Core/Misc/String.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.animation.SkeletonFormatLws", 0, SkeletonFormatLws, SkeletonFormat)

Ref< Skeleton > SkeletonFormatLws::create(LwsDocument* document) const
{
	LwsGroup* rootGroup = document->getRootGroup();
	T_ASSERT (rootGroup);

	Ref< Skeleton > skeleton = new Skeleton();
	Ref< Joint > current;

	std::map< std::wstring, int32_t > nameCount;
	std::map< int32_t, int32_t > mm;
	int32_t id = 0;
	
	float channels[8] = { 0.0f };

	uint32_t count = rootGroup->getCount();
	for (uint32_t i = 0; i < count; ++i)
	{
		const LwsNode* node = rootGroup->get(i);
		T_ASSERT (node);

		const LwsValue* value = dynamic_type_cast< const LwsValue* >(node);
		if (value)
		{
			if (value->getName() == L"AddBone" && value->getCount() >= 1)
			{
				if (current)
					mm[id] = skeleton->addJoint(current);

				current = new Joint();
				current->setRadius(0.1f);

				id = value->getInteger(0);
			}

			if (current != 0 && value->getName() == L"BoneName" && value->getCount() >= 1)
			{
				current->setName(value->getString(0));
			}

			if (current != 0 && value->getName() == L"BoneWeightMapName" && value->getCount() >= 1)
			{
				std::wstring jointName = value->getString(0);

				int32_t jointNameIndex = nameCount[jointName]++;
				if (jointNameIndex > 0)
					jointName += L"_" + toString(jointNameIndex);

				current->setName(jointName);
			}

			if (current != 0 && value->getName() == L"Channel")
			{
				int32_t channelIndex = value->getInteger(0);
				if (channelIndex >= 0 && channelIndex < sizeof_array(channels))
				{
					const LwsGroup* env = dynamic_type_cast< const LwsGroup* >(rootGroup->get(i + 1));
					if (!env)
						return 0;

					const LwsValue* key = dynamic_type_cast< const LwsValue* >(env->find(L"Key"));
					if (key)
						channels[channelIndex] = key->getFloat(0);
				}
			}

			if (current != 0 && value->getName() == L"ParentItem" && value->getCount() >= 1)
			{
				int32_t parent = value->getInteger(0);

				std::map< int32_t, int32_t >::const_iterator it = mm.find(parent);
				if (it != mm.end())
					current->setParent(it->second);

				float h = channels[3 + 0];
				float p = channels[3 + 1];
				float b = channels[3 + 2];

				Transform T(
					Vector4(channels[0], channels[1], channels[2], 0.0f),
					Quaternion::fromEulerAngles(0.0f, 0.0f, b) *
					Quaternion::fromEulerAngles(0.0f, p, 0.0f) *
					Quaternion::fromEulerAngles(h, 0.0f, 0.0f)
				);
				current->setTransform(T);

				mm[id] = skeleton->addJoint(current);
				current = 0;
			}
		}
	}

	if (current)
		skeleton->addJoint(current);

	return skeleton;
}

void SkeletonFormatLws::getExtensions(std::wstring& outDescription, std::vector< std::wstring >& outExtensions) const
{
	outDescription = L"Lightwave Scene";
	outExtensions.push_back(L"lws");
}

bool SkeletonFormatLws::supportFormat(const std::wstring& extension) const
{
	return compareIgnoreCase< std::wstring >(extension, L"lws") == 0;
}

Ref< Skeleton > SkeletonFormatLws::read(IStream* stream, const Vector4& offset, float scale, float radius, bool invertX, bool invertZ) const
{
	Ref< LwsDocument > document = LwsDocument::parse(stream);
	if (!document)
		return 0;

	return create(document);
}

	}
}
