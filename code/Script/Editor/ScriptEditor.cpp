#include "Script/Editor/ScriptEditor.h"
#include "Script/Script.h"
#include "Ui/Custom/SyntaxRichEdit/SyntaxRichEdit.h"
#include "Ui/Custom/SyntaxRichEdit/SyntaxLanguageLua.h"
#include "Core/Heap/HeapNew.h"

namespace traktor
{
	namespace script
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.script.ScriptEditor", ScriptEditor, editor::ObjectEditor)

bool ScriptEditor::create(ui::Widget* parent, db::Instance* instance, Object* object)
{
	m_script = dynamic_type_cast< Script* >(object);
	if (!m_script)
		return false;

	m_edit = gc_new< ui::custom::SyntaxRichEdit >();
	if (!m_edit->create(parent, m_script->getText()))
		return false;

	m_edit->setFont(ui::Font(
		L"Courier New",
		16
	));

	m_edit->setLanguage(gc_new< ui::custom::SyntaxLanguageLua >());

	return true;
}

void ScriptEditor::destroy()
{
	m_edit->destroy();
}

void ScriptEditor::apply()
{
	m_script->setText(m_edit->getText());
}

	}
}
