#ifndef traktor_render_ShaderViewer_H
#define traktor_render_ShaderViewer_H

#include "Ui/Container.h"

namespace traktor
{
	namespace editor
	{

class IEditor;

	}

	namespace ui
	{
		namespace custom
		{

class SyntaxRichEdit;
class ToolBar;
class ToolBarDropDown;

		}
	}

	namespace render
	{

class ShaderGraph;

class ShaderViewer : public ui::Container
{
	T_RTTI_CLASS;

public:
	ShaderViewer(editor::IEditor* editor);

	bool create(ui::Widget* parent);

	void reflect(const ShaderGraph* shaderGraph);

private:
	editor::IEditor* m_editor;
	Ref< ui::custom::ToolBar > m_shaderTools;
	Ref< ui::custom::ToolBarDropDown > m_compilerTool;
	Ref< ui::custom::SyntaxRichEdit > m_shaderEdit;
	Ref< ShaderGraph > m_shaderGraph;

	void updateViews();

	void eventShaderToolsClick(ui::Event* event);
};

	}
}

#endif	// traktor_render_ShaderViewer_H
