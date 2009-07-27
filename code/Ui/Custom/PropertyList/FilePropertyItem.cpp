#include "Ui/Custom/PropertyList/FilePropertyItem.h"
#include "Ui/Custom/PropertyList/PropertyList.h"
#include "Ui/Custom/MiniButton.h"
#include "Ui/Command.h"
#include "Ui/MethodHandler.h"
#include "Core/Io/StringOutputStream.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.FilePropertyItem", FilePropertyItem, PropertyItem)

FilePropertyItem::FilePropertyItem(const std::wstring& text, const Path& path)
:	PropertyItem(text)
,	m_path(path)
{
}

void FilePropertyItem::setPath(const Path& path)
{
	m_path = path;
}

const Path& FilePropertyItem::getPath() const
{
	return m_path;
}

void FilePropertyItem::createInPlaceControls(Widget* parent, bool visible)
{
	m_buttonEdit = gc_new< MiniButton >();
	m_buttonEdit->create(parent, L"...");
	m_buttonEdit->setVisible(visible);
	m_buttonEdit->addClickEventHandler(createMethodHandler(this, &FilePropertyItem::eventClick));
}

void FilePropertyItem::destroyInPlaceControls()
{
	m_buttonEdit->destroy();
}

void FilePropertyItem::resizeInPlaceControls(const Rect& rc, std::vector< WidgetRect >& outChildRects)
{
	outChildRects.push_back(WidgetRect(
		m_buttonEdit,
		Rect(
			rc.right - rc.getHeight(),
			rc.top,
			rc.right,
			rc.bottom
		)
	));
}

void FilePropertyItem::showInPlaceControls(bool show)
{
	m_buttonEdit->setVisible(show);
}

void FilePropertyItem::paintValue(Canvas& canvas, const Rect& rc)
{
	canvas.drawText(rc.inflate(-2, -2), m_path, AnLeft, AnCenter);
}

void FilePropertyItem::eventClick(Event* event)
{
	notifyCommand(Command(L"Property.Edit"));
}

		}
	}
}
