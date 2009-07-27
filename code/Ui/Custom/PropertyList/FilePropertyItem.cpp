#include "Ui/Custom/PropertyList/FilePropertyItem.h"
#include "Ui/Custom/PropertyList/PropertyList.h"
#include "Ui/Custom/MiniButton.h"
#include "Ui/Edit.h"
#include "Ui/Command.h"
#include "Ui/MethodHandler.h"
#include "Ui/Events/FocusEvent.h"
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
	m_editor = gc_new< Edit >();
	m_editor->create(
		parent,
		L"",
		WsNone
	);
	m_editor->setVisible(false);
	m_editor->addFocusEventHandler(createMethodHandler(this, &FilePropertyItem::eventEditFocus));

	m_buttonEdit = gc_new< MiniButton >();
	m_buttonEdit->create(parent, L"...");
	m_buttonEdit->setVisible(visible);
	m_buttonEdit->addClickEventHandler(createMethodHandler(this, &FilePropertyItem::eventClick));
}

void FilePropertyItem::destroyInPlaceControls()
{
	m_buttonEdit->destroy();
	m_editor->destroy();
}

void FilePropertyItem::resizeInPlaceControls(const Rect& rc, std::vector< WidgetRect >& outChildRects)
{
	outChildRects.push_back(WidgetRect(
		m_editor,
		Rect(
			rc.left,
			rc.top,
			rc.right - rc.getHeight(),
			rc.bottom
		)
	));
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

void FilePropertyItem::mouseButtonDown(MouseEvent* event)
{
	m_editor->setText(m_path);
	m_editor->setVisible(true);
	m_editor->setFocus();
	m_editor->selectAll();
}

void FilePropertyItem::paintValue(Canvas& canvas, const Rect& rc)
{
	canvas.drawText(rc.inflate(-2, -2), m_path, AnLeft, AnCenter);
}

void FilePropertyItem::eventEditFocus(Event* event)
{
	FocusEvent* f = static_cast< FocusEvent* >(event);
	if (f->lostFocus())
	{
		m_path = m_editor->getText();
		m_editor->setVisible(false);
		notifyChange();
	}
}

void FilePropertyItem::eventClick(Event* event)
{
	notifyCommand(Command(L"Property.Edit"));
}

		}
	}
}
