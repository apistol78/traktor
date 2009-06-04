#include "Editor/App/AboutDialog.h"
#include "Ui/TableLayout.h"
#include "Ui/Bitmap.h"
#include "Ui/Image.h"
#include "I18N/Text.h"
#include "Core/Heap/New.h"

// Resources
#include "Resources/About.h"

namespace traktor
{
	namespace editor
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.AboutDialog", AboutDialog, ui::Dialog)

bool AboutDialog::create(ui::Widget* parent)
{
	if (!ui::Dialog::create(parent, i18n::Text(L"EDITOR_ABOUT_TITLE"), 0, 0, ui::Dialog::WsDefaultFixed, gc_new< ui::TableLayout >(L"*,*", L"*", 0, 0)))
		return false;

	Ref< ui::Image > image = gc_new< ui::Image >();
	if (!image->create(this, ui::Bitmap::load(c_ResourceAbout, sizeof(c_ResourceAbout), L"png")))
		return false;

	fit();

	return true;
}

	}
}
