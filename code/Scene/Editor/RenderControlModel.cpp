/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Log/Log.h"
#include "Database/Database.h"
#include "Editor/IEditor.h"
#include "Scene/Editor/EntityAdapter.h"
#include "Scene/Editor/IModifier.h"
#include "Scene/Editor/ISceneRenderControl.h"
#include "Scene/Editor/RenderControlModel.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "Scene/Editor/TransformChain.h"
#include "Ui/Application.h"
#include "Ui/Widget.h"

namespace traktor::scene
{
	namespace
	{

int32_t translateMouseButton(int32_t uimb)
{
	if (uimb == ui::MbtLeft)
		return 1;
	else if (uimb == ui::MbtRight)
		return 2;
	else if (uimb == (ui::MbtLeft | ui::MbtRight) || uimb == ui::MbtMiddle)
		return 3;
	else
		return 0;
}

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.scene.RenderControlModel", RenderControlModel, Object)

RenderControlModel::RenderControlModel()
:	m_mouseButton(0)
,	m_modifyBegun(false)
,	m_modifyAlternative(false)
,	m_modifyClone(false)
,	m_modify(MtNothing)
,	m_moveCamera(0)
,	m_movementSpeed(40.0f)
{
}

void RenderControlModel::update(ISceneRenderControl* renderControl, ui::Widget* renderWidget, SceneEditorContext* context, const TransformChain& transformChain)
{
	if (m_mouseButton == 0 || m_moveCamera == 0)
		return;

	Vector4 delta = Vector4::zero();
	if ((m_moveCamera & (1 << 0)) != 0)
		delta += Vector4(0.0f, 0.0f, 1.0f);
	if ((m_moveCamera & (1 << 1)) != 0)
		delta += Vector4(0.0f, 0.0f, -1.0f);
	if ((m_moveCamera & (1 << 2)) != 0)
		delta += Vector4(-1.0f, 0.0f, 0.0f);
	if ((m_moveCamera & (1 << 3)) != 0)
		delta += Vector4(1.0f, 0.0f, 0.0f);
	if ((m_moveCamera & (1 << 4)) != 0)
		delta += Vector4(0.0f, 1.0f, 0.0f);
	if ((m_moveCamera & (1 << 5)) != 0)
		delta += Vector4(0.0f, -1.0f, 0.0f);

	const float deltaTime = std::min< float >(m_timer.getDeltaTime(), 1.0f / 30.0f);
	Scalar speed = Scalar(deltaTime * m_movementSpeed) * 8.0_simd;

	const uint32_t keyState = ui::Application::getInstance()->getEventLoop()->getAsyncKeyState();
	if ((keyState & ui::KsShift) != 0)
		speed *= 4.0_simd;
	if ((keyState & ui::KsControl) != 0)
		speed /= 4.0_simd;

	renderControl->moveCamera(ISceneRenderControl::McmMove, delta * speed, Vector4::zero());

	context->enqueueRedraw(renderControl);
}

void RenderControlModel::eventButtonDown(ISceneRenderControl* renderControl, ui::Widget* renderWidget, ui::MouseButtonDownEvent* event, SceneEditorContext* context, const TransformChain& transformChain)
{
	m_mousePosition0 =
	m_mousePosition = event->getPosition();
	m_mouseButton = translateMouseButton(event->getButton());

	m_modify = MtNothing;
	m_modifyAlternative = false;
	m_modifyClone = false;
	m_modifyBegun = false;

	T_ASSERT(m_modify == MtNothing);

	const ui::Rect innerRect = renderWidget->getInnerRect();
	const Vector2 screenPosition(2.0f * float(m_mousePosition.x) / innerRect.getWidth() - 1.0f, 1.0f - 2.0f * float(m_mousePosition.y) / innerRect.getHeight());

	if (m_mouseButton == 2)
		m_modify = MtCamera;
	if ((event->getKeyState() & ui::KsControl) != 0)
		m_modifyAlternative = true;
	if ((event->getKeyState() & (ui::KsControl | ui::KsShift)) == (ui::KsControl | ui::KsShift))
		m_modifyClone = true;

	if (m_modify != MtCamera)
	{
		m_modify = MtModifier;

		bool modifierHit = false;

		IModifier* modifier = context->getModifier();
		if (modifier)
		{
			Vector4 worldRayOrigin, worldRayDirection;

			renderControl->calculateRay(
				m_mousePosition,
				worldRayOrigin,
				worldRayDirection
			);

			modifier->buttonDown();

			modifierHit = modifier->cursorMoved(
				transformChain,
				screenPosition,
				worldRayOrigin,
				worldRayDirection
			).hot;
		}

		// Handle entity picking if enabled.
		if (
			!m_modifyClone &&
			m_mouseButton == 1 &&
			context->getPickEnable()
		)
		{
			if (!modifierHit && (event->getKeyState() & ui::KsShift) == 0)
				m_modify = MtSelection;
		}

		// Ensure modifier is hit when cloning.
		if (m_modifyClone)
		{
			if (!modifierHit)
				m_modifyClone = false;
		}

		// Ensure controller isn't playing and physics is disabled.
		context->setPlaying(false);
		context->setPhysicsEnable(false);
	}

	renderWidget->setCapture();
	renderWidget->setFocus();

	context->enqueueRedraw(renderControl);
}

void RenderControlModel::eventButtonUp(ISceneRenderControl* renderControl, ui::Widget* renderWidget, ui::MouseButtonUpEvent* event, SceneEditorContext* context, const TransformChain& transformChain)
{
	if (m_modify == MtModifier && m_modifyBegun)
	{
		IModifier* modifier = context->getModifier();
		if (modifier)
			modifier->end(transformChain);

		context->raisePostModify();
	}

	if (m_modify == MtSelection)
	{
		const ui::Rect selectionRectangle = ui::Rect(m_mousePosition0, m_mousePosition).getUnified();
		if (selectionRectangle.area() > 0)
		{
			// Selection rectangle was drawn; use frustum query to find new selection set.
			Frustum worldFrustum;
			if (renderControl->calculateFrustum(selectionRectangle, worldFrustum))
			{
				RefArray< EntityAdapter > intersectingEntities = context->queryFrustum(worldFrustum, true);

				// De-select all other if shift isn't held.
				if ((event->getKeyState() & (ui::KsShift | ui::KsControl)) == 0)
					context->selectAllEntities(false);

				// Remove selection if ctrl is begin held.
				for (auto intersectingEntity : intersectingEntities)
				{
					if ((event->getKeyState() & ui::KsControl) == 0)
						context->selectEntity(intersectingEntity, true);
					else
						context->selectEntity(intersectingEntity, false);
				}

				context->raiseSelect();
			}
		}
		else
		{
			// Single clicked; use ray query to find a single entity selection.
			Vector4 worldRayOrigin, worldRayDirection;
			if (renderControl->calculateRay(m_mousePosition, worldRayOrigin, worldRayDirection))
			{
				Ref< EntityAdapter > entityAdapter = context->queryRay(worldRayOrigin, worldRayDirection, true, true);

				// De-select all other if shift isn't held.
				if ((event->getKeyState() & (ui::KsShift | ui::KsControl)) == 0)
					context->selectAllEntities(false);

				if (entityAdapter)
				{
					// Toggle selection if ctrl is being held.
					if ((event->getKeyState() & ui::KsControl) == 0)
						context->selectEntity(entityAdapter, true);
					else
						context->selectEntity(entityAdapter, !entityAdapter->isSelected());

					context->raiseSelect();
				}
			}
		}

		renderControl->showSelectionRectangle(ui::Rect());
	}

	m_mousePosition0 =
	m_mousePosition = ui::Point(0, 0);
	m_mouseButton = 0;

	m_modify = MtNothing;
	m_modifyAlternative = false;
	m_modifyClone = false;
	m_modifyBegun = false;

	m_moveCamera = 0;

	if (renderWidget->hasCapture())
		renderWidget->releaseCapture();

	renderWidget->update();
}

void RenderControlModel::eventDoubleClick(ISceneRenderControl* renderControl, ui::Widget* renderWidget, ui::MouseDoubleClickEvent* event, SceneEditorContext* context, const TransformChain& transformChain)
{
}

void RenderControlModel::eventMouseMove(ISceneRenderControl* renderControl, ui::Widget* renderWidget, ui::MouseMoveEvent* event, SceneEditorContext* context, const TransformChain& transformChain)
{
	const ui::Point mousePosition = event->getPosition();

	const Vector4 mouseDelta(
		float(m_mousePosition.x - mousePosition.x),
		float(m_mousePosition.y - mousePosition.y),
		0.0f,
		0.0f
	);

	const ui::Rect innerRect = renderWidget->getInnerRect();
	const Vector2 screenPosition(2.0f * float(mousePosition.x) / innerRect.getWidth() - 1.0f, 1.0f - 2.0f * float(mousePosition.y) / innerRect.getHeight());
	const Vector4 clipDelta = mouseDelta * Vector4(-2.0f / innerRect.getWidth(), 2.0f / innerRect.getHeight(), 0.0f, 0.0f);
	const Vector4 viewDelta = transformChain.clipToView(clipDelta);

	bool needRedraw = false;

	if (m_modify == MtModifier)
	{
		const bool snapOverrideEnable = ((event->getKeyState() & ui::KsControl) != 0);

		IModifier* modifier = context->getModifier();
		T_ASSERT(modifier);

		Vector4 worldRayOrigin, worldRayDirection;
		renderControl->calculateRay(
			m_mousePosition,
			worldRayOrigin,
			worldRayDirection
		);

		if (!m_modifyBegun)
		{
			// Clone selection set; clones will become selected.
			if (m_modifyClone)
				context->cloneSelected();

			// Notify modifier about modification begun.
			if (modifier)
			{
				const Vector2 screenPosition0(
					2.0f * float(m_mousePosition0.x) / innerRect.getWidth() - 1.0f,
					1.0f - 2.0f * float(m_mousePosition0.y) / innerRect.getHeight()
				);

				modifier->cursorMoved(transformChain, screenPosition0, worldRayOrigin, worldRayDirection);
				modifier->cursorMoved(transformChain, screenPosition, worldRayOrigin, worldRayDirection);

				if (!modifier->begin(transformChain, screenPosition, worldRayOrigin, worldRayDirection, m_mouseButton))
				{
					m_modify = MtNothing;
					return;
				}
			}

			// Issue begin modification event.
			context->raisePreModify();

			m_modifyBegun = true;
		}

		modifier->apply(
			transformChain,
			screenPosition,
			worldRayOrigin,
			worldRayDirection,
			mouseDelta,
			viewDelta,
			snapOverrideEnable
		);

		needRedraw = true;
	}
	else if (m_modify == MtCamera)
	{
		if (m_mouseButton == 1)
			renderControl->moveCamera(ISceneRenderControl::McmMoveXZ, mouseDelta, viewDelta);
		else if (m_mouseButton == 2 && !m_modifyAlternative)
			renderControl->moveCamera(ISceneRenderControl::McmRotate, mouseDelta, viewDelta);
		else if (m_mouseButton == 3 || (m_mouseButton == 2 && m_modifyAlternative))
			renderControl->moveCamera(ISceneRenderControl::McmMoveXY, mouseDelta, viewDelta);

		context->raiseCameraMoved();
		needRedraw = true;
	}
	else if (m_modify == MtSelection)
	{
		renderControl->showSelectionRectangle(
			ui::Rect(m_mousePosition0, mousePosition).getUnified()
		);
		needRedraw = true;
	}
	else
	{
		IModifier* modifier = context->getModifier();
		if (modifier)
		{
			Vector4 worldRayOrigin, worldRayDirection;

			renderControl->calculateRay(
				m_mousePosition,
				worldRayOrigin,
				worldRayDirection
			);

			needRedraw = modifier->cursorMoved(
				transformChain,
				screenPosition,
				worldRayOrigin,
				worldRayDirection
			).redraw;
		}
	}

	if (needRedraw)
		context->enqueueRedraw(renderControl);

	m_mousePosition = mousePosition;
}

void RenderControlModel::eventKeyDown(ISceneRenderControl* renderControl, ui::Widget* renderWidget, ui::KeyDownEvent* event, SceneEditorContext* context, const TransformChain& transformChain)
{
	if (m_mouseButton == 0)
		return;

	const uint32_t lastMoveCamera = m_moveCamera;

	switch (event->getVirtualKey())
	{
	case ui::VkW:
		m_moveCamera |= (1 << 0);
		break;

	case ui::VkS:
		m_moveCamera |= (1 << 1);
		break;

	case ui::VkA:
		m_moveCamera |= (1 << 2);
		break;

	case ui::VkD:
		m_moveCamera |= (1 << 3);
		break;

	case ui::VkQ:
		m_moveCamera |= (1 << 4);
		break;

	case ui::VkE:
		m_moveCamera |= (1 << 5);
		break;

	default:
		return;
	}

	if (lastMoveCamera != m_moveCamera)
	{
		// Just issue this to ensure first update isn't too large.
		m_timer.getDeltaTime();
		context->enqueueRedraw(renderControl);
	}

	event->consume();
}

void RenderControlModel::eventKeyUp(ISceneRenderControl* renderControl, ui::Widget* renderWidget, ui::KeyUpEvent* event, SceneEditorContext* context, const TransformChain& transformChain)
{
	if (m_mouseButton == 0)
		return;

	switch (event->getVirtualKey())
	{
	case ui::VkW:
		m_moveCamera &= ~(1 << 0);
		break;

	case ui::VkS:
		m_moveCamera &= ~(1 << 1);
		break;

	case ui::VkA:
		m_moveCamera &= ~(1 << 2);
		break;

	case ui::VkD:
		m_moveCamera &= ~(1 << 3);
		break;

	case ui::VkQ:
		m_moveCamera &= ~(1 << 4);
		break;

	case ui::VkE:
		m_moveCamera &= ~(1 << 5);
		break;

	default:
		return;
	}

	context->enqueueRedraw(renderControl);
	event->consume();
}

}
