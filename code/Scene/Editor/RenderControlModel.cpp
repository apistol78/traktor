#include "Database/Database.h"
#include "Editor/IEditor.h"
#include "Scene/Editor/EntityAdapter.h"
#include "Scene/Editor/IModifier.h"
#include "Scene/Editor/ISceneRenderControl.h"
#include "Scene/Editor/RenderControlModel.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "Scene/Editor/TransformChain.h"
#include "Ui/Widget.h"
#include "Ui/Events/MouseEvent.h"

namespace traktor
{
	namespace scene
	{
		namespace
		{

int32_t translateMouseButton(int32_t uimb)
{
	if (uimb == ui::MouseEvent::BtLeft)
		return 1;
	else if (uimb == ui::MouseEvent::BtRight)
		return 2;
	else if (uimb == (ui::MouseEvent::BtLeft | ui::MouseEvent::BtRight) || uimb == ui::MouseEvent::BtMiddle)
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
{
}

void RenderControlModel::eventButtonDown(ISceneRenderControl* renderControl, ui::Widget* renderWidget, ui::Event* event, SceneEditorContext* context, const TransformChain& transformChain)
{
	ui::MouseEvent* mouseEvent = checked_type_cast< ui::MouseEvent*, false >(event);

	m_mousePosition0 =
	m_mousePosition = mouseEvent->getPosition();
	m_mouseButton = translateMouseButton(mouseEvent->getButton());

	m_modify = MtNothing;
	m_modifyAlternative = false;
	m_modifyClone = false;
	m_modifyBegun = false;

	T_ASSERT (m_modify == MtNothing);

	ui::Rect innerRect = renderWidget->getInnerRect();
	Vector2 screenPosition(2.0f * float(m_mousePosition.x) / innerRect.getWidth() - 1.0f, 1.0f - 2.0f * float(m_mousePosition.y) / innerRect.getHeight());

	if ((mouseEvent->getKeyState() & ui::KsMenu) != 0)
		m_modify = MtCamera;
	if ((mouseEvent->getKeyState() & ui::KsControl) != 0)
		m_modifyAlternative = true;
	if ((mouseEvent->getKeyState() & (ui::KsControl | ui::KsShift)) == (ui::KsControl | ui::KsShift))
		m_modifyClone = true;

	if (m_modify != MtCamera)
	{
		m_modify = MtModifier;

		// Handle entity picking if enabled.
		if (
			!m_modifyAlternative &&
			m_mouseButton == 1 &&
			context->getPickEnable()
		)
		{
			IModifier* modifier = context->getModifier();
			bool modifierHit = false;

			// Check if current modifier is being hit; modifier should always take precedence.
			if (modifier)
			{
				// Should be able to "press through" modifier if shift is held.
				if ((event->getKeyState() & ui::KsShift) == 0)
					modifierHit = modifier->cursorMoved(
						transformChain,
						screenPosition
					);
			}

			if (!modifierHit)
				m_modify = MtSelection;
		}

		// Ensure controller isn't playing and physics is disabled.
		context->setPlaying(false);
		context->setPhysicsEnable(false);
	}

	renderWidget->setCapture();
	renderWidget->setFocus();
}

void RenderControlModel::eventButtonUp(ISceneRenderControl* renderControl, ui::Widget* renderWidget, ui::Event* event, SceneEditorContext* context, const TransformChain& transformChain)
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
		ui::Rect selectionRectangle = ui::Rect(m_mousePosition0, m_mousePosition).getUnified();
		if (selectionRectangle.area() > 0)
		{
			// Selection rectangle was drawn; use frustum query to find new selection set.
			Frustum worldFrustum;
			if (renderControl->calculateFrustum(selectionRectangle, worldFrustum))
			{
				RefArray< EntityAdapter > intersectingEntities;
				context->queryFrustum(worldFrustum, intersectingEntities, true);

				// De-select all other if shift isn't held.
				if ((event->getKeyState() & ui::KsShift) == 0)
					context->selectAllEntities(false);

				for (RefArray< EntityAdapter >::iterator i = intersectingEntities.begin(); i != intersectingEntities.end(); ++i)
					context->selectEntity(*i);
				context->raiseSelect(this);
			}
		}
		else
		{
			// Single clicked; use ray query to find a single entity selection.
			Vector4 worldRayOrigin, worldRayDirection;
			if (renderControl->calculateRay(m_mousePosition, worldRayOrigin, worldRayDirection))
			{
				Ref< EntityAdapter > entityAdapter = context->queryRay(worldRayOrigin, worldRayDirection, true);

				// De-select all other if shift isn't held.
				if ((event->getKeyState() & ui::KsShift) == 0)
					context->selectAllEntities(false);

				context->selectEntity(entityAdapter);
				context->raiseSelect(this);
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

	if (renderWidget->hasCapture())
		renderWidget->releaseCapture();
}

void RenderControlModel::eventDoubleClick(ISceneRenderControl* renderControl, ui::Widget* renderWidget, ui::Event* event, SceneEditorContext* context, const TransformChain& transformChain)
{
	ui::MouseEvent* mouseEvent = checked_type_cast< ui::MouseEvent*, false >(event);
	ui::Point mousePosition = mouseEvent->getPosition();

	if (mouseEvent->getButton() != ui::MouseEvent::BtLeft)
		return;
	
	Vector4 worldRayOrigin, worldRayDirection;
	if (renderControl->calculateRay(m_mousePosition, worldRayOrigin, worldRayDirection))
	{
		Ref< EntityAdapter > entityAdapter = context->queryRay(worldRayOrigin, worldRayDirection, true);
		if (entityAdapter && entityAdapter->isExternal())
		{
			Guid externalGuid;
			entityAdapter->getExternalGuid(externalGuid);

			Ref< db::Instance > instance = context->getEditor()->getSourceDatabase()->getInstance(externalGuid);
			if (instance)
				context->getEditor()->openEditor(instance);
		}
	}
}

void RenderControlModel::eventMouseMove(ISceneRenderControl* renderControl, ui::Widget* renderWidget, ui::Event* event, SceneEditorContext* context, const TransformChain& transformChain)
{
	ui::MouseEvent* mouseEvent = checked_type_cast< ui::MouseEvent*, false >(event);
	ui::Point mousePosition = mouseEvent->getPosition();

	Vector4 mouseDelta(
		float(m_mousePosition.x - mousePosition.x),
		float(m_mousePosition.y - mousePosition.y),
		0.0f,
		0.0f
	);

	ui::Rect innerRect = renderWidget->getInnerRect();
	Vector2 screenPosition(2.0f * float(mousePosition.x) / innerRect.getWidth() - 1.0f, 1.0f - 2.0f * float(mousePosition.y) / innerRect.getHeight());
	Vector4 clipDelta = mouseDelta * Vector4(-2.0f / innerRect.getWidth(), 2.0f / innerRect.getHeight(), 0.0f, 0.0f);
	Vector4 viewDelta = transformChain.clipToView(clipDelta);

	if (m_modify == MtModifier)
	{
		IModifier* modifier = context->getModifier();
		T_ASSERT (modifier);

		if (!m_modifyBegun)
		{
			// Clone selection set; clones will become selected.
			if (m_modifyClone)
				context->cloneSelected();

			// Notify modifier about modification begun.
			if (modifier)
			{
				modifier->cursorMoved(transformChain, screenPosition);
				modifier->begin(transformChain);
			}

			// Issue begin modification event.
			context->raisePreModify();

			m_modifyBegun = true;
		}

		modifier->apply(
			transformChain,
			mouseDelta,
			viewDelta
		);
	}
	else if (m_modify == MtCamera)
	{
		if (m_mouseButton == 1)
			renderControl->moveCamera(ISceneRenderControl::McmRotate, mouseDelta, viewDelta);
		else if (m_mouseButton == 2 && !m_modifyAlternative)
			renderControl->moveCamera(ISceneRenderControl::McmMoveXZ, mouseDelta, viewDelta);
		else if (m_mouseButton == 3 || (m_mouseButton == 2 && m_modifyAlternative))
			renderControl->moveCamera(ISceneRenderControl::McmMoveXY, mouseDelta, viewDelta);
	}
	else if (m_modify == MtSelection)
	{
		renderControl->showSelectionRectangle(
			ui::Rect(m_mousePosition0, mousePosition).getUnified()
		);
	}
	else
	{
		IModifier* modifier = context->getModifier();
		if (modifier)
			modifier->cursorMoved(transformChain, screenPosition);
	}

	renderWidget->update();

	m_mousePosition = mousePosition;
}

	}
}
