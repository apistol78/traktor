/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <limits>
#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Reflection/Reflection.h"
#include "Core/Reflection/RfmArray.h"
#include "Core/Reflection/RfmObject.h"
#include "Core/Settings/PropertyColor.h"
#include "Core/Settings/PropertyGroup.h"
#include "Editor/IEditor.h"
#include "Render/PrimitiveRenderer.h"
#include "Scene/Editor/DefaultEntityEditor.h"
#include "Scene/Editor/EntityAdapter.h"
#include "Scene/Editor/IModifier.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "Ui/Command.h"
#include "World/EntityData.h"
#include "World/Entity/FacadeComponentData.h"
#include "World/Entity/GroupComponentData.h"
#include "World/Entity/LightComponentData.h"
#include "World/Entity/VolumeComponentData.h"

namespace traktor::scene
{
	namespace
	{

template < typename ComponentDataType >
ComponentDataType* getComponentOf(const world::EntityData* entityData)
{
	for (auto component : entityData->getComponents())
	{
		if (auto typedComponent = dynamic_type_cast< ComponentDataType* >(component))
			return typedComponent;
	}
	return nullptr;
}

bool projectSphere(const Vector4& center, const Scalar& radius, const Scalar& znear, const Scalar& P00, const Scalar& P11, Vector4& aabb)
{
	if (center.z() < radius + znear)
		return false;

	Vector4 cr = center * radius;
	Scalar czr2 = center.z() * center.z() - radius * radius;

	Scalar vx = squareRoot(center.x() * center.x() + czr2);
	Scalar minx = (vx * center.x() - cr.z()) / (vx * center.z() + cr.x());
	Scalar maxx = (vx * center.x() + cr.z()) / (vx * center.z() - cr.x());

	Scalar vy = squareRoot(center.y() * center.y() + czr2);
	Scalar miny = (vy * center.y() - cr.z()) / (vy * center.z() + cr.y());
	Scalar maxy = (vy * center.y() + cr.z()) / (vy * center.z() - cr.y());

	aabb = Vector4(minx * P00, miny * P11, maxx * P00, maxy * P11);
	//aabb = aabb.xwzy * vec4(0.5f, -0.5f, 0.5f, -0.5f) + vec4(0.5f); // clip space -> uv space
	aabb = aabb.shuffle< 0, 3, 2, 1 >() * Vector4(0.5f, -0.5f, 0.5f, -0.5f) + Vector4(0.5f, 0.5f, 0.5f, 0.5f);

	return true;
}

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.scene.DefaultEntityEditor", DefaultEntityEditor, IEntityEditor)

DefaultEntityEditor::DefaultEntityEditor(SceneEditorContext* context, EntityAdapter* entityAdapter)
:	m_context(context)
,	m_entityAdapter(entityAdapter)
,	m_inModify(false)
{
	updateSettings();
}

bool DefaultEntityEditor::isPickable() const
{
	const world::EntityData* entityData = m_entityAdapter->getEntityData();
	if (getComponentOf< world::VolumeComponentData >(entityData) != nullptr)
		return false;

	return !isGroup();
}

bool DefaultEntityEditor::isGroup() const
{
	const world::EntityData* entityData = m_entityAdapter->getEntityData();
	if (getComponentOf< world::GroupComponentData >(entityData) != nullptr)
		return true;
	else if (getComponentOf< world::FacadeComponentData >(entityData) != nullptr)
		return true;
	else
		return false;
}

bool DefaultEntityEditor::isChildrenPrivate() const
{
	return false;
}

bool DefaultEntityEditor::addChildEntity(EntityAdapter* insertAfterEntityAdapter, EntityAdapter* childEntityAdapter) const
{
	world::EntityData* entityData = m_entityAdapter->getEntityData();
	if (auto groupComponentData = getComponentOf< world::GroupComponentData >(entityData))
	{
		groupComponentData->addEntityData(
			insertAfterEntityAdapter ? insertAfterEntityAdapter->getEntityData() : nullptr,
			childEntityAdapter->getEntityData()
		);
		return true;
	}
	else
		return false;
}

bool DefaultEntityEditor::removeChildEntity(EntityAdapter* childEntityAdapter) const
{
	world::EntityData* entityData = m_entityAdapter->getEntityData();
	world::EntityData* childEntityData = childEntityAdapter->getEntityData();

	RefArray< Reflection > rs;
	rs.push_back(Reflection::create(entityData));
	for (auto componentData : entityData->getComponents())
		rs.push_back(Reflection::create(componentData));

	uint32_t removedCount = 0;
	for (auto r : rs)
	{
		for (uint32_t i = 0; i < r->getMemberCount(); ++i)
		{
			RfmObject* ro = dynamic_type_cast< RfmObject* >(r->getMember(i));
			if (ro != nullptr && ro->get() == childEntityData)
			{
				ro->set(nullptr);
				removedCount++;
			}

			RfmArray* ra = dynamic_type_cast< RfmArray* >(r->getMember(i));
			if (ra != nullptr)
			{
				for (uint32_t j = 0; j < ra->getMemberCount(); ++j)
				{
					RfmObject* ro = dynamic_type_cast< RfmObject* >(ra->getMember(j));
					if (ro != nullptr && ro->get() == childEntityData)
					{
						ra->removeMember(ro);
						removedCount++;
					}
				}
			}
		}
	}

	if (removedCount > 0)
	{
		auto it = rs.begin();
		(*it++)->apply(entityData);
		for (auto componentData : entityData->getComponents())
			(*it++)->apply(componentData);
		return true;
	}

	return false;
}

bool DefaultEntityEditor::queryRay(const Vector4& worldRayOrigin, const Vector4& worldRayDirection, Scalar& outDistance) const
{
	// Transform ray into object space.
	const Transform worldInv = m_entityAdapter->getTransform().inverse();
	const Vector4 objectRayOrigin = worldInv * worldRayOrigin.xyz1();
	const Vector4 objectRayDirection = worldInv * worldRayDirection.xyz0();

	// Get entity bounding box; do not pick if origin of ray is within box.
	const Aabb3 boundingBox = m_entityAdapter->getBoundingBox();
	if (boundingBox.empty() || boundingBox.inside(objectRayOrigin))
		return false;

	// Trace bounding box to see if ray intersect.
	const Scalar length = outDistance - Scalar(FUZZY_EPSILON);
	Scalar distance;
	if (!boundingBox.intersectSegment(objectRayOrigin, objectRayOrigin + objectRayDirection * length, distance))
		return false;

	T_FATAL_ASSERT (distance <= 1.0f);
	outDistance = distance * length;
	return true;
}

bool DefaultEntityEditor::queryFrustum(const Frustum& worldFrustum) const
{
	// Transform frustum into object space.
	const Transform worldInv = m_entityAdapter->getTransform().inverse();
	Plane objectPlanes[6];
	for (int32_t i = 0; i < 6; ++i)
		objectPlanes[i] = worldInv.toMatrix44() * worldFrustum.planes[i];

	// Get entity bounding box.
	const Aabb3 boundingBox = m_entityAdapter->getBoundingBox();
	if (boundingBox.empty())
		return false;

	Frustum objectFrustum;
	objectFrustum.buildFromPlanes(objectPlanes);
	return objectFrustum.inside(boundingBox) != Frustum::Result::Outside;
}

void DefaultEntityEditor::entitySelected(bool selected)
{
}

bool DefaultEntityEditor::handleCommand(const ui::Command& command)
{
	if (command == L"Editor.SettingsChanged")
		updateSettings();
	return false;
}

void DefaultEntityEditor::drawGuide(render::PrimitiveRenderer* primitiveRenderer, const ui::Size& clientSize) const
{
	const Vector4 c_expandBoundingBox(0.001f, 0.001f, 0.001f, 0.0f);

	const Transform transform = m_entityAdapter->getTransform();
	Aabb3 boundingBox = m_entityAdapter->getBoundingBox();
	boundingBox.mn -= c_expandBoundingBox;
	boundingBox.mx += c_expandBoundingBox;

	if (!m_entityAdapter->getParent())
		return;

	if (m_context->shouldDrawGuide(L"Entity.BoundingBox"))
	{
		primitiveRenderer->pushWorld(transform.toMatrix44());
		if (m_entityAdapter->isSelected())
		{
			primitiveRenderer->drawSolidAabb(boundingBox, m_colorBoundingBoxFaceSel);
			primitiveRenderer->drawWireAabb(boundingBox, 3.0f, m_colorBoundingBoxSel);
		}
		else
			primitiveRenderer->drawWireAabb(boundingBox, 1.0f, m_colorBoundingBox);
		primitiveRenderer->popWorld();

		// if (m_entityAdapter->isSelected())
		// {
		// 	Vector4 center = (boundingBox.mn + boundingBox.mx) / 2.0_simd;
		// 	//Scalar radius = ((boundingBox.mx - boundingBox.mn) / 2.0_simd).length();

		// 	Scalar radius = max(boundingBox.mx.length(), boundingBox.mn.length());

		// 	Vector4 worldCenter = transform.translation() + center;
		// 	Vector4 viewCenter = primitiveRenderer->getView() * worldCenter.xyz1();

		// 	Scalar znear = 0.1_simd;
		// 	Scalar P00 = primitiveRenderer->getProjection().get(0, 0);
		// 	Scalar P11 = primitiveRenderer->getProjection().get(1, 1);

		// 	Vector4 aabb;
		// 	if (projectSphere(viewCenter, radius, znear, P00, P11, aabb))
		// 	{
		// 		Matrix44 proj = primitiveRenderer->getProjection();

		// 		primitiveRenderer->pushWorld(Matrix44::identity());
		// 		primitiveRenderer->pushView(Matrix44::identity());

		// 		// Setup projection as UV space.
		// 		primitiveRenderer->setProjection(orthoLh(
		// 			-1.0f, 1.0f,
		// 			1.0f, -1.0f,
		// 			-1.0f, 1.0f
		// 		) * translate(-1.0f, -1.0f, 0.0f) * scale(2.0f, 2.0f, 1.0f));

		// 		const float w = (aabb.z() - aabb.x()) * clientSize.cx;
		// 		const float h = (aabb.w() - aabb.y()) * clientSize.cy;

		// 		const int32_t level = int32_t(std::floor(std::log2(std::max(w, h))));
		// 		const float step = 4096.0f / std::max(4096 >> level, 1);
		// 		for (float y = 0.0f; y < clientSize.cy; y += step)
		// 		{
		// 			const float fy = float(y) / clientSize.cy;
		// 			primitiveRenderer->drawLine(
		// 				Vector4(0.0f, fy, 0.0f, 1.0f),
		// 				Vector4(1.0f, fy, 0.0f, 1.0f),
		// 				Color4ub(255, 255, 0, 120)
		// 			);
		// 		}
		// 		for (float x = 0.0f; x < clientSize.cx; x += step)
		// 		{
		// 			const float fx = float(x) / clientSize.cx;
		// 			primitiveRenderer->drawLine(
		// 				Vector4(fx, 0.0f, 0.0f, 1.0f),
		// 				Vector4(fx, 1.0f, 0.0f, 1.0f),
		// 				Color4ub(255, 255, 0, 120)
		// 			);
		// 		}

		// 		primitiveRenderer->drawLine(Vector4(aabb.x(), aabb.y(), 0.0f, 1.0f), Vector4(aabb.z(), aabb.y(), 0.0f, 1.0f), Color4ub(255, 0, 0, 255));
		// 		primitiveRenderer->drawLine(Vector4(aabb.x(), aabb.w(), 0.0f, 1.0f), Vector4(aabb.z(), aabb.w(), 0.0f, 1.0f), Color4ub(255, 0, 0, 255));
		// 		primitiveRenderer->drawLine(Vector4(aabb.x(), aabb.y(), 0.0f, 1.0f), Vector4(aabb.x(), aabb.w(), 0.0f, 1.0f), Color4ub(255, 0, 0, 255));
		// 		primitiveRenderer->drawLine(Vector4(aabb.z(), aabb.y(), 0.0f, 1.0f), Vector4(aabb.z(), aabb.w(), 0.0f, 1.0f), Color4ub(255, 0, 0, 255));

		// 		primitiveRenderer->setProjection(proj);
		// 		primitiveRenderer->popView();
		// 		primitiveRenderer->popWorld();
		// 	}
		// }

		if (m_entityAdapter->isSelected() && m_context->getSnapMode() == SceneEditorContext::SmNeighbour)
		{
			for (const auto& snapPoint : m_entityAdapter->getSnapPoints())
			{
				primitiveRenderer->drawSolidPoint(
					snapPoint.position,
					4.0f,
					m_colorSnap
				);
			}
		}
	}
}

bool DefaultEntityEditor::getStatusText(std::wstring& outStatusText) const
{
	return false;
}

void DefaultEntityEditor::updateSettings()
{
	Ref< PropertyGroup > colors = m_context->getEditor()->getSettings()->getProperty< PropertyGroup >(L"Editor.Colors");
	m_colorBoundingBox = colors->getProperty< Color4ub >(L"BoundingBoxWire");
	m_colorBoundingBoxSel = colors->getProperty< Color4ub >(L"BoundingBoxWireSelected");
	m_colorBoundingBoxFaceSel = colors->getProperty< Color4ub >(L"BoundingBoxFaceSelected");
	m_colorSnap = colors->getProperty< Color4ub >(L"SnapPoint");
}

}
