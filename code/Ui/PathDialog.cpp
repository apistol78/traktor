#include "Core/Log/Log.h"
#include "Ui/Application.h"
#include "Ui/PathDialog.h"
#include "Ui/Widget.h"
#include "Ui/Itf/IPathDialog.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.PathDialog", PathDialog, EventSubject)

PathDialog::PathDialog()
:	m_pathDialog(0)
{
}

PathDialog::~PathDialog()
{
	T_ASSERT_M (!m_pathDialog, L"PathDialog not destroyed");
}

bool PathDialog::create(Widget* parent, const std::wstring& title)
{
	m_pathDialog = Application::getInstance()->getWidgetFactory()->createPathDialog(this);
	if (!m_pathDialog)
	{
		log::error << L"Failed to create native widget peer (PathDialog)" << Endl;
		return false;
	}

	if (!m_pathDialog->create(parent ? parent->getIWidget() : 0, title))
		return false;

	return true;
}

void PathDialog::destroy()
{
	if (m_pathDialog)
	{
		m_pathDialog->destroy();
		m_pathDialog = 0;
	}
}

int PathDialog::showModal(Path& outPath)
{
	T_ASSERT (m_pathDialog);
	return m_pathDialog->showModal(outPath);
}

	}
}
