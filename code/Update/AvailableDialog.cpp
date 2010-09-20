#include "Core/Misc/Split.h"
#include "Ui/Bitmap.h"
#include "Ui/Image.h"
#include "Ui/ListBox.h"
//#include "Ui/Static.h"
#include "Ui/TableLayout.h"
#include "Update/AvailableDialog.h"
#include "Update/Bundle.h"

// Resources
#include "Resources/UpdateBanner.h"

namespace traktor
{
	namespace update
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.update.AvailableDialog", AvailableDialog, ui::ConfigDialog)

bool AvailableDialog::create(const Bundle* bundle)
{
	if (!ui::ConfigDialog::create(
		0,
		L"Update",
		600,
		500,
		ui::Dialog::WsDefaultResizable,
		new ui::TableLayout(L"100%", L"*,100%", 0, 0)
	))
		return false;

	Ref< ui::Image > imageBanner = new ui::Image();
	imageBanner->create(
		this,
		ui::Bitmap::load(c_ResourceUpdateBanner, sizeof(c_ResourceUpdateBanner), L"png"),
		ui::Image::WsTransparent
	);

	Ref< ui::ListBox > listDescription = new ui::ListBox();
	listDescription->create(this);

	std::vector< std::wstring > lines;
	Split< std::wstring >::any(bundle->getDescription(), L"\n\r", lines, true);
	for (std::vector< std::wstring >::const_iterator i = lines.begin(); i != lines.end(); ++i)
		listDescription->add(*i);

	return true;
}

	}
}
