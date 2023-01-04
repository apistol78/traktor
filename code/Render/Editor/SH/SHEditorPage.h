/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Ref.h"
#include "Editor/IEditorPage.h"
#include "Render/SH/SHCoeffs.h"
#include "Render/SH/SHMatrix.h"

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

class RenderControl;
class RenderControlEvent;
class SHEngine;
class SHFunction;

class T_DLLCLASS SHEditorPage : public editor::IEditorPage
{
	T_RTTI_CLASS;

public:
	explicit SHEditorPage(editor::IEditor* editor);

	virtual bool create(ui::Container* parent) override final;

	virtual void destroy() override final;

	virtual bool dropInstance(db::Instance* instance, const ui::Point& position) override final;

	virtual bool handleCommand(const ui::Command& command) override final;

	virtual void handleDatabaseEvent(db::Database* database, const Guid& eventId) override final;

private:
	editor::IEditor* m_editor;
	Ref< resource::IResourceManager > m_resourceManager;
	Ref< ui::Container > m_container;
	Ref< RenderControl > m_renderControls[2];
	Ref< SHEngine > m_engine;
	Ref< SHFunction > m_lightFunction;
	//Ref< SHFunction > m_phaseFunction;
	SHCoeffs m_lightCoefficients;
	//SHCoeffs m_phaseCoefficients;
	//SHMatrix m_phaseTransfer;

	void eventRender1(RenderControlEvent* event);

	void eventRender2(RenderControlEvent* event);
};

}
