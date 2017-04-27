/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_editor_ScriptNodeDialog_H
#define traktor_editor_ScriptNodeDialog_H

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

		namespace custom
		{

class GridItemContentChangeEvent;
class GridRowDoubleClickEvent;
class GridView;
class SyntaxRichEdit;

		}
	}

	namespace render
	{

class Script;

class ScriptNodeDialog : public ui::ConfigDialog
{
	T_RTTI_CLASS;

public:
	ScriptNodeDialog(editor::IEditor* editor, Script* script);

	bool create(ui::Widget* parent);

	std::wstring getText() const;

	int32_t getInputPinCount() const;

	std::wstring getInputPinName(int32_t index) const;

	ParameterType getInputPinType(int32_t index) const;

	int32_t getOutputPinCount() const;

	std::wstring getOutputPinName(int32_t index) const;

	ParameterType getOutputPinType(int32_t index) const;

private:
	editor::IEditor* m_editor;
	Ref< Script > m_script;
	Ref< ui::custom::GridView > m_inputPinList;
	Ref< ui::custom::GridView > m_outputPinList;
	Ref< ui::custom::SyntaxRichEdit > m_edit;
	
	void eventInputPinRowDoubleClick(ui::custom::GridRowDoubleClickEvent* event);

	void eventInputPinEdit(ui::custom::GridItemContentChangeEvent* event);

	void eventOutputPinRowDoubleClick(ui::custom::GridRowDoubleClickEvent* event);

	void eventOutputPinEdit(ui::custom::GridItemContentChangeEvent* event);
};

	}
}

#endif	// traktor_editor_ScriptNodeDialog_H
