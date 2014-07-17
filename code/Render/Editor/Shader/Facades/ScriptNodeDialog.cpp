#include "I18N/Text.h"
#include "Render/Shader/InputPin.h"
#include "Render/Shader/OutputPin.h"
#include "Render/Shader/Script.h"
#include "Render/Editor/Shader/Facades/ScriptNodeDialog.h"
#include "Ui/FloodLayout.h"
#include "Ui/ListBox.h"
#include "Ui/Tab.h"
#include "Ui/TabPage.h"
#include "Ui/Custom/Splitter.h"
#include "Ui/Custom/SyntaxRichEdit/SyntaxLanguageHlsl.h"
#include "Ui/Custom/SyntaxRichEdit/SyntaxRichEdit.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

const wchar_t* c_platforms[] =
{
	L"DX9",
	L"DX9 Xbox360",
	L"DX10",
	L"DX11",
	L"OpenGL",
	L"OpenGL ES2",
	L"GCM"
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ScriptNodeDialog", ScriptNodeDialog, ui::ConfigDialog)

ScriptNodeDialog::ScriptNodeDialog(Script* script)
:	m_script(script)
{
}

bool ScriptNodeDialog::create(ui::Widget* parent)
{
	if (!ui::ConfigDialog::create(parent, i18n::Text(L"SHADERGRAPH_SCRIPT_EDIT"), 800, 500, ui::ConfigDialog::WsDefaultResizable | ui::ConfigDialog::WsApplyButton, new ui::FloodLayout()))
		return false;

	addEventHandler< ui::ButtonClickEvent >(this, &ScriptNodeDialog::eventClick);

	Ref< ui::custom::Splitter > splitter = new ui::custom::Splitter();
	splitter->create(this, true, 150);

	Ref< ui::custom::Splitter > splitter2 = new ui::custom::Splitter();
	splitter2->create(splitter, false, -20, true);

	Ref< ui::ListBox > inputPinList = new ui::ListBox();
	inputPinList->create(splitter2);

	int32_t inputPinCount = m_script->getInputPinCount();
	for (int32_t i = 0; i < inputPinCount; ++i)
		inputPinList->add(m_script->getInputPin(i)->getName());

	Ref< ui::ListBox > outputPinList = new ui::ListBox();
	outputPinList->create(splitter2);

	int32_t outputPinCount = m_script->getOutputPinCount();
	for (int32_t i = 0; i < outputPinCount; ++i)
		outputPinList->add(m_script->getOutputPin(i)->getName());

	Ref< ui::Tab > tabPlatforms = new ui::Tab();
	tabPlatforms->create(splitter, ui::WsBorder);
	
	m_edit.resize(sizeof_array(c_platforms));
	for (uint32_t i = 0; i < sizeof_array(c_platforms); ++i)
	{
		Ref< ui::TabPage > tabPage = new ui::TabPage();
		tabPage->create(tabPlatforms, c_platforms[i], new ui::FloodLayout());

		m_edit[i] = new ui::custom::SyntaxRichEdit();
		if (!m_edit[i]->create(tabPage, m_script->getScript(c_platforms[i])))
			return false;

		m_edit[i]->setLanguage(new ui::custom::SyntaxLanguageHlsl());
		m_edit[i]->setFont(ui::Font(L"Courier New", 14));

		tabPlatforms->addPage(tabPage);
	}

	tabPlatforms->setActivePage(tabPlatforms->getPage(0));

	update();
	return true;
}

void ScriptNodeDialog::eventClick(ui::ButtonClickEvent* event)
{
	const ui::Command& command = event->getCommand();
	if (command.getId() == ui::DrOk || command.getId() == ui::DrApply)
	{
		for (uint32_t i = 0; i < sizeof_array(c_platforms); ++i)
		{
			std::wstring script = m_edit[i]->getText();
			m_script->setScript(c_platforms[i], script);
		}
	}
}

	}
}
