#include "Core/Log/Log.h"
#include "Ui/Gtk/UserWidgetGtk.h"

namespace traktor
{
	namespace ui
	{

UserWidgetGtk::UserWidgetGtk(EventSubject* owner)
:	WidgetGtkImpl< IUserWidget >(owner)
{
}

bool UserWidgetGtk::create(IWidget* parent, int style)
{
	Warp* p = static_cast< Warp* >(parent->getInternalHandle());
	T_FATAL_ASSERT(p != nullptr);

	m_warp.widget = t_bare_container_new();
	gtk_container_add(GTK_CONTAINER(p->widget), m_warp.widget);

	if (!WidgetGtkImpl< IUserWidget >::create(parent))
		return false;

	GtkStyleContext* cx = gtk_widget_get_style_context(m_warp.widget);
	if (cx == nullptr)
		return false;

	const PangoFontDescription* fd = gtk_style_context_get_font(cx, GTK_STATE_FLAG_NORMAL);
	if (fd == nullptr)
		return false;

	m_font = Font(
		mbstows(pango_font_description_get_family(fd)),
		pango_font_description_get_size(fd) / PANGO_SCALE + 2,
		false,
		false,
		false
	);

	g_signal_connect(m_warp.widget, "draw", G_CALLBACK(UserWidgetGtk::signal_draw), this);
	return true;
}

void UserWidgetGtk::setFont(const Font& font)
{
	m_font = font;
}

Font UserWidgetGtk::getFont() const
{
	return m_font;
}

void UserWidgetGtk::getAscentAndDescent(int32_t& outAscent, int32_t& outDescent) const
{
	outAscent = 0;
	outDescent = 0;
}

int32_t UserWidgetGtk::getAdvance(wchar_t ch, wchar_t next) const
{
	return 0;
}

int32_t UserWidgetGtk::getLineSpacing() const
{
	return 0;
}

Size UserWidgetGtk::getExtent(const std::wstring& text) const
{
	int ew = 0, eh = 0;

	GtkStyleContext* cx = gtk_widget_get_style_context(m_warp.widget);
	if (cx == nullptr)
		return Size(0, 0);

	const PangoFontDescription* fd = gtk_style_context_get_font(cx, GTK_STATE_FLAG_NORMAL);
	if (fd == nullptr)
		return Size(0, 0);

	PangoContext* pcx = gtk_widget_get_pango_context(m_warp.widget);
	PangoLayout* ly = pango_layout_new(pcx);
	pango_layout_set_text(ly, wstombs(text).c_str(), -1);
	pango_layout_set_font_description(ly, fd);
	pango_layout_get_pixel_size(ly, &ew, &eh);
	g_object_unref(ly);

	return Size(ew, eh);
}

gboolean UserWidgetGtk::signal_draw(GtkWidget* widget, cairo_t* cr, gpointer data)
{
	UserWidgetGtk* self = static_cast< UserWidgetGtk* >(data);
	T_FATAL_ASSERT(self != nullptr);

	Rect rc = self->getInnerRect();

	CanvasGtk canvasGtk(cr);
	
	Canvas canvas(&canvasGtk);
	canvas.setFont(self->m_font);

	PaintEvent paintEvent(self->m_owner, canvas, rc);
	self->m_owner->raiseEvent(&paintEvent);

	return FALSE;
}

	}
}

