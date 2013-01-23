#include <map>
#include "Animation/Joint.h"
#include "Animation/Skeleton.h"
#include "Animation/Editor/SkeletonFormatLws.h"
#include "Animation/Editor/LwsParser/LwsDocument.h"
#include "Animation/Editor/LwsParser/LwsGroup.h"
#include "Animation/Editor/LwsParser/LwsValue.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.SkeletonFormatLws", SkeletonFormatLws, ISkeletonFormat)

Ref< Skeleton > SkeletonFormatLws::import(IStream* stream, const Vector4& offset, float radius, bool invertX, bool invertZ) const
{
	Ref< LwsDocument > document = LwsDocument::parse(stream);
	if (!document)
		return 0;

	LwsGroup* rootGroup = document->getRootGroup();
	T_ASSERT (rootGroup);

	Ref< Skeleton > skeleton = new Skeleton();
	Ref< Joint > current;

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
				current->setName(value->getString(0));
			}

			if (current != 0 && value->getName() == L"NumChannels" && value->getCount() >= 1)
			{
				numChannels = value->getInteger(0);
			}

			if (current != 0 && numChannels > 0 && value->getName() == L"Channel" && value->getCount() >= 1)
			{
				int32_t channel = value->getInteger(0);

				const LwsGroup* group = dynamic_type_cast< const LwsGroup* >(rootGroup->get(i + 1));
				if (group && group->getName() == L"Envelope")
				{
					const LwsValue* key = dynamic_type_cast< const LwsValue* >(group->find(L"Key"));
					if (key && key->getCount() >= 1)
					{
						Transform T = current->getTransform();

						float kv = key->getFloat(0);
						switch (channel)
						{
						// X
						case 0:
							T = Transform(
								T.translation() + Vector4(kv, 0.0f, 0.0f, 0.0f),
								T.rotation()
							);
							break;

						// Y
						case 1:
							T = Transform(
								T.translation() + Vector4(0.0f, kv, 0.0f, 0.0f),
								T.rotation()
							);
							break;

						// Z
						case 2:
							T = Transform(
								T.translation() + Vector4(0.0f, 0.0f, kv, 0.0f),
								T.rotation()
							);
							break;

						// Head
						case 3:
							T = Transform(
								T.translation(),
								Quaternion::fromEulerAngles(kv, 0.0f, 0.0f) * T.rotation()
							);
							break;

						// Pitch
						case 4:
							T = Transform(
								T.translation(),
								Quaternion::fromEulerAngles(0.0f, kv, 0.0f) * T.rotation()
							);
							break;

						// Bank
						case 5:
							T = Transform(
								T.translation(),
								Quaternion::fromEulerAngles(0.0f, 0.0f, kv) * T.rotation()
							);
							break;
						}

						current->setTransform(T);
					}
				}

				--numChannels;
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

	}
}
