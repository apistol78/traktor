#include "Ui/Edit.h"
#include "Ui/Static.h"
#include "Ui/TableLayout.h"
#include "Update/AvailableDialog.h"
#include "Update/Bundle.h"

namespace traktor
{
	namespace update
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.update.AvailableDialog", AvailableDialog, ui::ConfigDialog)

bool AvailableDialog::create(const Bundle* bundle)
{
	if (!ui::ConfigDialog::create(
		0,
		L"Update available!",
		600,
		500,
		ui::Dialog::WsDefaultResizable,
		new ui::TableLayout(L"100%", L"*,100%", 4, 4)
	))
		return false;

	Ref< ui::Static > staticNotice = new ui::Static();
	staticNotice->create(this, L"New release available, press Ok to update now!");

	Ref< ui::Edit > editDescription = new ui::Edit();
	editDescription->create(this, bundle->getDescription());

	return true;
}

	}
}
