/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/Editor/RagDoll/RagDollSkeletonPipeline.h"

#include "Animation/Editor/RagDoll/RagDollBone.h"
#include "Animation/Editor/RagDoll/RagDollSkeletonAsset.h"
#include "Animation/RagDoll/RagDollSkeleton.h"
#include "Core/Containers/SmallMap.h"
#include "Core/Log/Log.h"
#include "Core/Serialization/DeepHash.h"
#include "Database/Instance.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineDepends.h"
#include "Physics/BallJointDesc.h"
#include "Physics/CollisionSpecification.h"
#include "Physics/ConeTwistJointDesc.h"
#include "Physics/DynamicBodyDesc.h"
#include "Physics/CapsuleShapeDesc.h"
#include "Physics/HingeJointDesc.h"
#include "Physics/JointDesc.h"

namespace traktor::animation
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.animation.RagDollSkeletonPipeline", 0, RagDollSkeletonPipeline, editor::IPipeline)

bool RagDollSkeletonPipeline::create(const editor::IPipelineSettings* settings, db::Database* database)
{
	return true;
}

void RagDollSkeletonPipeline::destroy()
{
}

TypeInfoSet RagDollSkeletonPipeline::getAssetTypes() const
{
	return makeTypeInfoSet< RagDollSkeletonAsset >();
}

uint32_t RagDollSkeletonPipeline::hashAsset(const ISerializable* sourceAsset) const
{
	return DeepHash(sourceAsset).get();
}

bool RagDollSkeletonPipeline::shouldCache() const
{
	return false;
}

bool RagDollSkeletonPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	const RagDollSkeletonAsset* asset = mandatory_non_null_type_cast< const RagDollSkeletonAsset* >(sourceAsset);

	for (const auto& id : asset->getCollisionGroup())
		pipelineDepends->addDependency(id, editor::PdfBuild | editor::PdfResource);
	for (const auto& id : asset->getCollisionMask())
		pipelineDepends->addDependency(id, editor::PdfBuild | editor::PdfResource);

	return true;
}

bool RagDollSkeletonPipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const editor::PipelineDependencySet* dependencySet,
	const editor::PipelineDependency* dependency,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid,
	const Object* buildParams,
	uint32_t reason
) const
{
	const RagDollSkeletonAsset* asset = mandatory_non_null_type_cast< const RagDollSkeletonAsset* >(sourceAsset);
	
	Ref< RagDollSkeleton > output = new RagDollSkeleton();

	// Map each bone to its limb index so parent and child limbs can be constrained.
	SmallMap< const RagDollBone*, int32_t > boneToLimb;

	asset->traverse([&](const RagDollBone* parent, const RagDollBone* bone, const Transform& tip) {

		const float length = bone->getLength();
		const float radius = bone->getRadius();

		// 'tip' is the frame at the parent's tip. Offset and orient by the bone's own
		// local transform to get its base frame, then offset half its length along
		// local Z (Traktor's capsule axis) to reach the capsule center.
		const Transform boneBase = tip * Transform(bone->getOffset(), bone->getRotation());
		const Transform boneCenter = boneBase * Transform(Vector4(0.0f, 0.0f, length * 0.5f, 0.0f));

		Ref< physics::CapsuleShapeDesc > shape = new physics::CapsuleShapeDesc();
		shape->setCollisionGroup(asset->getCollisionGroup());
		shape->setCollisionMask(asset->getCollisionMask());
		shape->setRadius(radius);
		shape->setLength(length);

		const int32_t limbIndex = (int32_t)output->m_limbs.size();

		RagDollSkeleton::Limb& limb = output->m_limbs.push_back();
		limb.transform = boneCenter;
		limb.body = new physics::DynamicBodyDesc(shape);
		limb.body->setMass(1.0f);
		limb.body->setLinearDamping(bone->getLinearDamping());
		limb.body->setAngularDamping(bone->getAngularDamping());

		for (const auto& infl : bone->getInfluences())
		{
			limb.influences.push_back({
				.joint = infl.joint,
				.weight = infl.weight
			});
		}

		boneToLimb[bone] = limbIndex;

		// Constrain this limb to its parent limb at this bone's base (offset from the
		// parent's tip). The bone's constraint description selects the joint type; the
		// anchor and axes are derived from the bone's base frame (model space).
		if (parent != nullptr)
		{
			const auto it = boneToLimb.find(parent);
			if (it != boneToLimb.end())
			{
				const RagDollBone::Constraint& constraint = bone->getConstraint();
				const Vector4 anchor = boneBase.translation().xyz1();

				Ref< physics::JointDesc > jointDesc;
				switch (constraint.type)
				{
				case RagDollBone::Constraint::Type::Cone:
					{
						// Swing about the bone axis (local Z) is limited by the cone; twist is about that axis.
						Ref< physics::ConeTwistJointDesc > coneTwist = new physics::ConeTwistJointDesc();
						coneTwist->setAnchor(anchor);
						coneTwist->setTwistAxis(boneBase.axisZ());
						coneTwist->setConeAxis(boneBase.axisX());
						coneTwist->setConeAngles(constraint.swingAngle, constraint.swingAngle);
						coneTwist->setTwistAngle(constraint.twistAngle);
						jointDesc = coneTwist;
					}
					break;

				case RagDollBone::Constraint::Type::Hinge:
					{
						// Hinge about the configured bone-local axis (its sign flips the bend direction).
						Vector4 hingeAxis;
						switch (constraint.hingeAxis)
						{
						case RagDollBone::Constraint::Axis::Y: hingeAxis = boneBase.axisY(); break;
						case RagDollBone::Constraint::Axis::Z: hingeAxis = boneBase.axisZ(); break;
						case RagDollBone::Constraint::Axis::NegX: hingeAxis = -boneBase.axisX(); break;
						case RagDollBone::Constraint::Axis::NegY: hingeAxis = -boneBase.axisY(); break;
						case RagDollBone::Constraint::Axis::NegZ: hingeAxis = -boneBase.axisZ(); break;
						default: hingeAxis = boneBase.axisX(); break;
						}

						Ref< physics::HingeJointDesc > hinge = new physics::HingeJointDesc();
						hinge->setAnchor(anchor);
						hinge->setAxis(hingeAxis);
						hinge->setEnableLimits(true);
						hinge->setAngles(constraint.minAngle, constraint.maxAngle);
						jointDesc = hinge;
					}
					break;

				default:
					{
						// Free; position constrained, rotation unconstrained.
						Ref< physics::BallJointDesc > ball = new physics::BallJointDesc();
						ball->setAnchor(anchor);
						jointDesc = ball;
					}
					break;
				}

				RagDollSkeleton::Joint& joint = output->m_joints.push_back();
				joint.joint = jointDesc;
				joint.body1 = it->second;
				joint.body2 = limbIndex;
			}
		}
	});

	Ref< db::Instance > instance = pipelineBuilder->createOutputInstance(outputPath, outputGuid);
	if (!instance)
	{
		log::error << L"Unable to build rag doll skeleton; unable to create output instance \"" << outputPath << L"\"." << Endl;
		return false;
	}

	instance->setObject(output);

	if (!instance->commit())
	{
		log::error << L"Unable to build rag doll skeleton; unable to commit output instance \"" << outputPath << L"\"." << Endl;
		return false;
	}

	return true;
}

Ref< ISerializable > RagDollSkeletonPipeline::buildProduct(
	editor::IPipelineBuilder* pipelineBuilder,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const Object* buildParams
) const
{
	T_FATAL_ERROR;
	return nullptr;
}

}
