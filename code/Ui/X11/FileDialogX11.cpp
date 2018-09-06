#include "Ui/Enums.h"
#include "Ui/X11/FileDialogX11.h"

namespace traktor
{
	namespace ui
	{

FileDialogX11::FileDialogX11(EventSubject* owner)
:	m_owner(owner)
{
}

FileDialogX11::~FileDialogX11()
{
}

bool FileDialogX11::create(IWidget* parent, const std::wstring& title, const std::wstring& filters, bool save)
{
	return true;
}

void FileDialogX11::destroy()
{
}

int FileDialogX11::showModal(Path& outPath)
{
	outPath = L"/home/apistol/private/traktor/Traktor.workspace";
	return DrOk;
}

int FileDialogX11::showModal(std::vector< Path >& outPaths)
{
	return DrCancel;
}

	}
}
