/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
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

namespace traktor
{
	namespace editor
	{

class IEditor;

	}

	namespace resource
	{

class IResourceManager;

	}

	namespace render
	{

class RenderControl;
class RenderControlEvent;
class SHEngine;
class SHFunction;

class T_DLLCLASS SHEditorPage : public editor::IEditorPage
{
	T_RTTI_CLASS;

public:
	SHEditorPage(editor::IEditor* editor);

	virtual bool create(ui::Container* parent) T_OVERRIDE T_FINAL;

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual bool dropInstance(db::Instance* instance, const ui::Point& position) T_OVERRIDE T_FINAL;

	virtual bool handleCommand(const ui::Command& command) T_OVERRIDE T_FINAL;

	virtual void handleDatabaseEvent(db::Database* database, const Guid& eventId) T_OVERRIDE T_FINAL;

private:
	editor::IEditor* m_editor;
	Ref< resource::IResourceManager > m_resourceManager;
	Ref< ui::Container > m_container;
	Ref< RenderControl > m_renderControls[4];
	Ref< SHEngine > m_engine;
	Ref< SHFunction > m_lightFunction;
	Ref< SHFunction > m_phaseFunction;
	SHCoeffs m_lightCoefficients;
	SHCoeffs m_phaseCoefficients;
	SHMatrix m_phaseTransfer;

	void eventRender1(RenderControlEvent* event);

	void eventRender2(RenderControlEvent* event);

	void eventRender3(RenderControlEvent* event);

	void eventRender4(RenderControlEvent* event);
};
		
	}
}
