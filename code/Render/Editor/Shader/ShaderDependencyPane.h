#pragma once

#include "Render/Editor/Shader/ShaderDependencyTracker.h"
#include "Ui/Container.h"

namespace traktor
{
	namespace editor
	{

class IEditor;

	}

	namespace ui
	{

class ListBox;

	}

	namespace render
	{

class ShaderDependencyTracker;

class ShaderDependencyPane
:	public ui::Container
,	public ShaderDependencyTracker::IListener
{
	T_RTTI_CLASS;

public:
	ShaderDependencyPane(editor::IEditor* editor, const Guid& shaderId);

	virtual void destroy() override final;

	bool create(ui::Widget* parent);

private:
	editor::IEditor* m_editor;
	Guid m_shaderId;
	Ref< ShaderDependencyTracker > m_dependencyTracker;
	Ref< ui::ListBox > m_refereeList;

	virtual void dependencyAdded(const Guid& fromShader, const Guid& toShader) override final;

	virtual void dependencyRemoved(const Guid& fromShader, const Guid& toShader) override final;

	void eventRefereeListDoubleClick(ui::MouseDoubleClickEvent* event);
};

	}
}

