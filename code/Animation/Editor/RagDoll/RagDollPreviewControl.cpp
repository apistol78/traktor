/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/Editor/RagDoll/RagDollPreviewControl.h"

#include "Animation/Editor/SkeletonRenderer.h"
#include "Animation/Editor/RagDoll/RagDollBone.h"
#include "Animation/Editor/RagDoll/RagDollSkeletonAsset.h"
#include "Core/Containers/SmallMap.h"
#include "Core/Math/Color4ub.h"
#include "Core/Math/Const.h"
#include "Core/Math/Matrix44.h"
#include "Core/Math/Transform.h"
#include "Physics/CapsuleShapeDesc.h"
#include "Render/Editor/RenderControlEvent.h"
#include "Render/PrimitiveRenderer.h"

namespace traktor::animation
{
	namespace
	{

const float c_coneLength = 0.15f;
const float c_arcRadius = 0.09f;
const float c_axisHalfLength = 0.1f;
const float c_freeRadius = 0.03f;
const float c_selectionMargin = 0.015f;
const Color4ub c_selectionColor(255, 180, 40, 255);

Vector4 hingeAxisVector(const RagDollBone::Constraint& constraint, const Transform& boneBase)
{
	switch (constraint.hingeAxis)
	{
	case RagDollBone::Constraint::Axis::Y:
		return boneBase.axisY();
	case RagDollBone::Constraint::Axis::Z:
		return boneBase.axisZ();
	case RagDollBone::Constraint::Axis::NegX:
		return -boneBase.axisX();
	case RagDollBone::Constraint::Axis::NegY:
		return -boneBase.axisY();
	case RagDollBone::Constraint::Axis::NegZ:
		return -boneBase.axisZ();
	default:
		return boneBase.axisX();
	}
}

// Draw the physics constraint connecting the bone (at boneBase) to its parent.
void drawConstraint(render::PrimitiveRenderer* primitiveRenderer, const Transform& boneBase, const RagDollBone::Constraint& constraint)
{
	const Vector4 anchor = boneBase.translation().xyz1();

	switch (constraint.type)
	{
	case RagDollBone::Constraint::Type::Cone:
		{
			// Swing cone opens along the bone (twist) axis; twist shown as an arc in the perpendicular plane.
			primitiveRenderer->drawCone(
				boneBase.toMatrix44(),
				constraint.swingAngle,
				constraint.swingAngle,
				c_coneLength,
				Color4ub(255, 220, 0, 64),
				Color4ub(255, 220, 0, 180)
			);

			if (constraint.twistAngle > FUZZY_EPSILON)
			{
				const float twistHalf = constraint.twistAngle * 0.5f;
				primitiveRenderer->drawProtractor(
					anchor,
					boneBase.axisX(),
					boneBase.axisY(),
					-twistHalf,
					twistHalf,
					deg2rad(4.0f),
					c_arcRadius,
					Color4ub(255, 140, 0, 64),
					Color4ub(255, 140, 0, 180)
				);
			}
		}
		break;

	case RagDollBone::Constraint::Type::Hinge:
		{
			// Hinge axis and the allowed swing arc from the bone's rest direction.
			const Vector4 axis = hingeAxisVector(constraint, boneBase);

			primitiveRenderer->drawLine(
				anchor - axis * Scalar(c_axisHalfLength),
				anchor + axis * Scalar(c_axisHalfLength),
				Color4ub(0, 255, 255, 220)
			);

			const Vector4 base = boneBase.axisZ();
			const Vector4 zero = cross(axis, base).normalized();
			primitiveRenderer->drawProtractor(
				anchor,
				base,
				zero,
				constraint.minAngle,
				constraint.maxAngle,
				deg2rad(4.0f),
				c_arcRadius,
				Color4ub(0, 220, 220, 64),
				Color4ub(0, 220, 220, 180)
			);
		}
		break;

	default:
		{
			// Free; indicate unconstrained rotation with a small sphere.
			primitiveRenderer->drawWireSphere(translate(anchor), c_freeRadius, Color4ub(160, 160, 160, 200));
		}
		break;
	}
}

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.RagDollPreviewControl", RagDollPreviewControl, render::RenderControl)

bool RagDollPreviewControl::create(ui::Widget* parent, render::IRenderSystem* renderSystem, db::Database* database)
{
	if (!render::RenderControl::create(parent, renderSystem, database))
		return false;

	// Draw our content each time the shared render loop renders a frame.
	addEventHandler< render::RenderControlEvent >(this, &RagDollPreviewControl::eventRender);
	return true;
}

void RagDollPreviewControl::setAsset(const RagDollSkeletonAsset* asset)
{
	m_asset = asset;
	m_selectedBone = nullptr;
	update();
}

void RagDollPreviewControl::setSkeleton(const Skeleton* skeleton)
{
	m_skeleton = skeleton;
	update();
}

void RagDollPreviewControl::setSelectedBone(const RagDollBone* bone)
{
	m_selectedBone = bone;
	update();
}

void RagDollPreviewControl::eventRender(render::RenderControlEvent* event)
{
	render::PrimitiveRenderer* primitiveRenderer = event->getPrimitiveRenderer();

	// Ground grid for orientation.
	for (int32_t i = -10; i <= 10; ++i)
	{
		const float f = float(i) * 0.5f;
		const Color4ub axisColor = (i == 0) ? Color4ub(120, 120, 120, 255) : Color4ub(60, 60, 60, 255);
		primitiveRenderer->drawLine(Vector4(f, 0.0f, -5.0f, 1.0f), Vector4(f, 0.0f, 5.0f, 1.0f), axisColor);
		primitiveRenderer->drawLine(Vector4(-5.0f, 0.0f, f, 1.0f), Vector4(5.0f, 0.0f, f, 1.0f), axisColor);
	}

	// Optional animation skeleton overlay.
	if (m_skeleton)
		drawSkeleton(primitiveRenderer, m_skeleton, Matrix44::identity(), Color4ub(255, 255, 0, 250));

	// Rag doll setup; evaluate the bind pose directly from the asset.
	if (m_asset)
	{
		SmallMap< const RagDollBone*, Transform > boneCenters;
		m_asset->traverse([&](const RagDollBone* parent, const RagDollBone* bone, const Transform& tip) {
			const float length = bone->getLength();
			const float radius = bone->getRadius();

			const Transform boneBase = tip * Transform(bone->getOffset(), bone->getRotation());
			const Transform boneCenter = boneBase * Transform(Vector4(0.0f, 0.0f, length * 0.5f, 0.0f));
			boneCenters[bone] = boneCenter;

			physics::CapsuleShapeDesc shapeDesc;
			shapeDesc.setRadius(radius);
			shapeDesc.setLength(length);

			const Transform bodyTransform[] = { boneCenter, boneCenter };
			m_physicsRenderer.draw(getResourceManager(), primitiveRenderer, bodyTransform, &shapeDesc);

			// Highlight the bone selected in the editor tree with a bright, slightly
			// enlarged capsule outline drawn over the standard one.
			if (bone == m_selectedBone)
				primitiveRenderer->drawWireCylinder(
					boneCenter.toMatrix44(),
					radius + c_selectionMargin,
					length,
					c_selectionColor
				);

			// Constraint at this bone's base.
			const bool selected = (bone == m_selectedBone);
			primitiveRenderer->drawSolidPoint(boneBase.translation().xyz1(), selected ? 10.0f : 6.0f, selected ? c_selectionColor : Color4ub(80, 160, 255, 255));
			if (parent != nullptr)
			{
				const auto it = boneCenters.find(parent);
				if (it != boneCenters.end())
					primitiveRenderer->drawLine(it->second.translation().xyz1(), boneBase.translation().xyz1(), Color4ub(80, 160, 255, 255));

				// Visualize the constraint (joint) linking this bone to its parent.
				drawConstraint(primitiveRenderer, boneBase, bone->getConstraint());
			}
		});
	}
}

}
