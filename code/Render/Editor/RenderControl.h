/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Math/Color4ub.h"
#include "Core/Math/Const.h"
#include "Core/Math/Matrix44.h"
#include "Core/Math/Vector4.h"
#include "Ui/Widget.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::editor
{

class IEditor;

}

namespace traktor::resource
{

class IResourceManager;

}

namespace traktor::render
{

class IRenderSystem;
class IRenderView;
class PrimitiveRenderer;

/*! 3d preview control.
 * \ingroup Render
 *
 * Owns the embedded render view, the primitive renderer and the camera
 * navigation which is shared by all editor 3d preview controls; left mouse
 * button moves the camera, holding control switch between moving in X/Z and
 * X/Y direction, right mouse button rotates.
 *
 * The default frame draws the primitive renderer content only, raising a
 * RenderControlEvent in which the owner draw its content; controls which
 * need more, such as a world renderer, override renderFrame instead.
 */
class T_DLLCLASS RenderControl : public ui::Widget
{
	T_RTTI_CLASS;

public:
	/*! Camera placement, kept in view space, i.e. position X and Y pan the
	 * camera across the screen and position Z move it to and from the scene.
	 */
	struct View
	{
		Vector4 position = Vector4::origo();
		float head = 0.0f;
		float pitch = 0.0f;
	};

	/*! Create control; a resource manager is created, and owned, for the editor's output database. */
	bool create(ui::Widget* parent, editor::IEditor* editor);

	/*! Create control using an already existing resource manager. */
	bool create(ui::Widget* parent, editor::IEditor* editor, resource::IResourceManager* resourceManager);

	virtual void destroy() override;

	void setPerspective(float fieldOfView, float nearZ, float farZ);

	void setView(const View& view);

	const View& getView() const { return m_view; }

	Matrix44 getViewTransform() const;

	/*! Draw ground grid in the XZ plane, centered at origin.
	 *
	 * The center axis lines are distinguished by color only, all lines are
	 * drawn equally thin; colors are themed through the stylesheet.
	 *
	 * \param primitiveRenderer Renderer to draw with.
	 * \param extent Half size of the grid.
	 * \param step Distance between grid lines.
	 */
	void drawGrid(PrimitiveRenderer* primitiveRenderer, float extent, float step) const;

	resource::IResourceManager* getResourceManager() const { return m_resourceManager; }

	IRenderSystem* getRenderSystem() const { return m_renderSystem; }

	IRenderView* getRenderView() const { return m_renderView; }

	PrimitiveRenderer* getPrimitiveRenderer() const { return m_primitiveRenderer; }

	uint32_t getMultiSample() const { return m_multiSample; }

	/*! Size of the render view; valid after validateRenderView has succeeded. */
	const ui::Size& getRenderSize() const { return m_renderSize; }

protected:
	/*! Consume render view events and reset the view if it has been lost or
	 * the widget resized; false if the view is unusable this frame.
	 */
	bool validateRenderView();

	/*! Render a frame, called from the paint event; false if nothing was rendered. */
	virtual bool renderFrame();

private:
	Ref< resource::IResourceManager > m_resourceManager;
	Ref< IRenderSystem > m_renderSystem;
	Ref< IRenderView > m_renderView;
	Ref< PrimitiveRenderer > m_primitiveRenderer;
	uint32_t m_multiSample = 0;
	bool m_ownResourceManager = false;
	ui::Size m_renderSize = ui::Size(0, 0);
	float m_fieldOfView = deg2rad(80.0f);
	float m_nearZ = 0.1f;
	float m_farZ = 2000.0f;
	View m_view;
	ui::Point m_lastMousePosition = ui::Point(0, 0);

	Color4ub getGridColor() const;

	Color4ub getGridAxisColor() const;

	bool createInternal(ui::Widget* parent, editor::IEditor* editor);

	void eventButtonDown(ui::MouseButtonDownEvent* event);

	void eventButtonUp(ui::MouseButtonUpEvent* event);

	void eventMouseMove(ui::MouseMoveEvent* event);

	void eventPaint(ui::PaintEvent* event);
};

}
