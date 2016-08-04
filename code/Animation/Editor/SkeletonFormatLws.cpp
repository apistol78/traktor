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

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.SkeletonFormatLws", SkeletonFormatLws, ISkeletonFormat)

Ref< Skeleton > SkeletonFormatLws::create(LwsDocument* document) const
{
	LwsGroup* rootGroup = document->getRootGroup();
	T_ASSERT (rootGroup);

	Ref< Skeleton > skeleton = new Skeleton();
	Ref< Joint > current;

	std::map< std::wstring, int32_t > nameCount;
	std::map< int32_t, int32_t > mm;
	int32_t id = 0;
	int32_t numChannels = 0;

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

			if (current != 0 && value->getName() == L"BoneRestPosition" && value->getCount() >= 3)
			{
				Transform T = current->getTransform();
				T = Transform(
					T.translation() + Vector4(value->getFloat(0), value->getFloat(1), value->getFloat(2), 0.0f),
					T.rotation()
				);
				current->setTransform(T);
			}

			if (current != 0 && value->getName() == L"BoneRestDirection" && value->getCount() >= 3)
			{
				Transform T = current->getTransform();
				T = Transform(
					T.translation(),
					Quaternion::fromEulerAngles(value->getFloat(0), 0.0f, 0.0f) *
					Quaternion::fromEulerAngles(0.0f, value->getFloat(1), 0.0f) *
					Quaternion::fromEulerAngles(0.0f, 0.0f, value->getFloat(2)) *
					T.rotation()
				);
				current->setTransform(T);
			}

			if (current != 0 && value->getName() == L"ParentItem" && value->getCount() >= 1)
			{
				int32_t parent = value->getInteger(0);
				
				std::map< int32_t, int32_t >::const_iterator it = mm.find(parent);
				if (it != mm.end())
					current->setParent(it->second);

				mm[id] = skeleton->addJoint(current);
				current = 0;
			}
		}
	}

	if (current)
		skeleton->addJoint(current);

	return skeleton;
}

Ref< Skeleton > SkeletonFormatLws::import(IStream* stream, const Vector4& offset, float scale, float radius, bool invertX, bool invertZ) const
{
	Ref< LwsDocument > document = LwsDocument::parse(stream);
	if (!document)
		return 0;

	return create(document);
}

	}
}
