#pragma once

#include "Core/RefArray.h"
#include "Ui/ConfigDialog.h"

namespace traktor
{
	namespace editor
	{

class IEditor;

	}

	namespace ui
	{

class ButtonClickEvent;
class GridItemContentChangeEvent;
class GridRowDoubleClickEvent;
class GridView;
class SyntaxRichEdit;

	}

	namespace render
	{

class Script;
class ShaderGraph;

class ScriptNodeDialog : public ui::ConfigDialog
{
	T_RTTI_CLASS;

public:
	ScriptNodeDialog(editor::IEditor* editor, Script* script);

	bool create(ui::Widget* parent);

	virtual std::wstring getText() const override final;

	int32_t getInputPinCount() const;

	std::wstring getInputPinName(int32_t index) const;

	ParameterType getInputPinType(int32_t index) const;

	int32_t getOutputPinCount() const;

	std::wstring getOutputPinName(int32_t index) const;

	ParameterType getOutputPinType(int32_t index) const;

	/*! Apply modifications on script node.
	 *
	 * \note
	 * Called after user has pressed Ok/Apply on dialog.
	 */
	bool apply(ShaderGraph* shaderGraph, Script* script, bool& outCompleteRebuild) const;

private:
	editor::IEditor* m_editor;
	Ref< Script > m_script;
	Ref< ui::GridView > m_inputPinList;
	Ref< ui::GridView > m_outputPinList;
	Ref< ui::SyntaxRichEdit > m_edit;

	void eventInputPinRowDoubleClick(ui::GridRowDoubleClickEvent* event);

	void eventInputPinEdit(ui::GridItemContentChangeEvent* event);

	void eventOutputPinRowDoubleClick(ui::GridRowDoubleClickEvent* event);

	void eventOutputPinEdit(ui::GridItemContentChangeEvent* event);
};

	}
}

