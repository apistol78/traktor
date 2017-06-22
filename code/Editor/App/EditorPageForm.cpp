#include "Editor/App/EditorPageForm.h"
#include "Ui/FloodLayout.h"

namespace traktor
{
	namespace editor
	{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.EditorPageForm", EditorPageForm, ui::Form)

bool EditorPageForm::create(ui::Widget* parent)
{
	if (!ui::Form::create(L"Editor Page", 1000, 1000, ui::Form::WsDefault, new ui::FloodLayout())) //, parent))
		return false;

	return true;
}

void EditorPageForm::destroy()
{
	ui::Form::destroy();
}

	}
}
