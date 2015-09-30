#include "Core/Io/StringOutputStream.h"
#include "Ui/Bitmap.h"
#include "Ui/Command.h"
#include "Ui/Edit.h"
#include "Ui/Custom/MiniButton.h"
#include "Ui/Custom/PropertyList/FilePropertyItem.h"
#include "Ui/Custom/PropertyList/PropertyList.h"

// Resources
#include "Resources/SmallDots.h"

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

void FilePropertyItem::createInPlaceControls(Widget* parent)
{
	T_ASSERT (!m_editor);
	m_editor = new Edit();
	m_editor->create(
		parent,
		L"",
		WsNone
	);
	m_editor->setVisible(false);
	m_editor->addEventHandler< FocusEvent >(this, &FilePropertyItem::eventEditFocus);

	T_ASSERT (!m_buttonEdit);
	m_buttonEdit = new MiniButton();
	m_buttonEdit->create(parent, ui::Bitmap::load(c_ResourceSmallDots, sizeof(c_ResourceSmallDots), L"png"));
	m_buttonEdit->addEventHandler< ButtonClickEvent >(this, &FilePropertyItem::eventClick);
}

void FilePropertyItem::destroyInPlaceControls()
{
	if (m_buttonEdit)
	{
		m_buttonEdit->destroy();
		m_buttonEdit = 0;
	}
	if (m_editor)
	{
		m_editor->destroy();
		m_editor = 0;
	}
}

void FilePropertyItem::resizeInPlaceControls(const Rect& rc, std::vector< WidgetRect >& outChildRects)
{
	if (m_editor)
		outChildRects.push_back(WidgetRect(
			m_editor,
			Rect(
				rc.left,
				rc.top,
				rc.right - rc.getHeight(),
				rc.bottom
			)
		));
	if (m_buttonEdit)
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

void FilePropertyItem::mouseButtonDown(MouseButtonDownEvent* event)
{
	m_editor->setText(m_path.getOriginal());
	m_editor->setVisible(true);
	m_editor->setFocus();
	m_editor->selectAll();
}

void FilePropertyItem::paintValue(Canvas& canvas, const Rect& rc)
{
	canvas.drawText(rc.inflate(-2, 0), m_path.getOriginal(), AnLeft, AnCenter);
}

void FilePropertyItem::eventEditFocus(FocusEvent* event)
{
	if (event->lostFocus())
	{
		m_path = m_editor->getText();
		m_editor->setVisible(false);
		notifyChange();
	}
}

void FilePropertyItem::eventClick(ButtonClickEvent* event)
{
	notifyCommand(Command(L"Property.Browse"));
}

		}
	}
}
