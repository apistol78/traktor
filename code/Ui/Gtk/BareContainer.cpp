#include <gtk/gtk.h>
#include "Core/Log/Log.h"
#include "Ui/Gtk/BareContainer.h"

using namespace traktor;

// Private class member
#define T_BARE_CONTAINER_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), T_BARE_CONTAINER_TYPE, BareContainerPrivate))

// Forward
static void t_bare_container_get_preferred_width(GtkWidget* widget, int* minimal, int* natural);
static void t_bare_container_get_preferred_height(GtkWidget* widget, int* minimal, int* natural);
static void t_bare_container_realize(GtkWidget* widget);
static void t_bare_container_size_allocate(GtkWidget* widget, GtkAllocation* allocation);
static gboolean t_bare_container_draw(GtkWidget* widget, cairo_t* cr);
static GType t_bare_container_child_type(GtkContainer* container);
static void t_bare_container_add(GtkContainer* container, GtkWidget* widget);
static void t_bare_container_remove(GtkContainer* container, GtkWidget* widget);
static void t_bare_container_forall(GtkContainer* container, gboolean includeInternals, GtkCallback callback, gpointer callbackData);

struct BareContainerPrivate
{
	GdkWindow* window;
	GList* children;
	int ox;
	int oy;
};

G_DEFINE_TYPE(BareContainer, t_bare_container, GTK_TYPE_CONTAINER);

static void t_bare_container_class_init(BareContainerClass* klass)
{
	GtkWidgetClass* widgetClass = GTK_WIDGET_CLASS(klass);
	widgetClass->get_preferred_width = t_bare_container_get_preferred_width;
	widgetClass->get_preferred_height = t_bare_container_get_preferred_height;
	widgetClass->realize = t_bare_container_realize;
	widgetClass->size_allocate = t_bare_container_size_allocate;
	//widgetClass->draw = t_bare_container_draw;

	GtkContainerClass* containerClass = GTK_CONTAINER_CLASS(klass);
	containerClass->child_type = t_bare_container_child_type;
	containerClass->add = t_bare_container_add;
	containerClass->remove = t_bare_container_remove;
	containerClass->forall = t_bare_container_forall;

	g_type_class_add_private(klass, sizeof(BareContainerPrivate));
}

static void t_bare_container_init(BareContainer* self)
{
	gtk_widget_set_has_window(GTK_WIDGET(self), TRUE);

	BareContainerPrivate* pr = T_BARE_CONTAINER_PRIVATE(self);
	pr->window = nullptr;
	pr->children = nullptr;
	pr->ox = 0;
	pr->oy = 0;
}

GtkWidget* t_bare_container_new()
{
	return GTK_WIDGET(g_object_new(t_bare_container_get_type(), NULL));
}

void t_base_container_place_child(BareContainer* container, GtkWidget* child, int x, int y, int width, int height)
{
	BareContainerPrivate* pr = T_BARE_CONTAINER_PRIVATE(container);
	T_FATAL_ASSERT(pr != nullptr);

	GtkAllocation a;
	a.x = pr->ox + x;
	a.y = pr->oy + y;
	a.width = width;
	a.height = height;
	gtk_widget_size_allocate(child, &a);
}

void t_base_container_get_child_position(BareContainer* container, GtkWidget* child, int* x, int* y)
{
	BareContainerPrivate* pr = T_BARE_CONTAINER_PRIVATE(container);
	T_FATAL_ASSERT(pr != nullptr);

	GtkAllocation a;
	gtk_widget_get_allocation(child, &a);

	*x = a.x - pr->ox;
	*y = a.y - pr->oy;
}

// Overridden methods

static void t_bare_container_get_preferred_width(GtkWidget* widget, int* minimal, int* natural)
{
	if (minimal)
		*minimal = 0;
	if (natural)
		*natural = 0;
}

static void t_bare_container_get_preferred_height(GtkWidget* widget, int* minimal, int* natural)
{
	if (minimal)
		*minimal = 0;
	if (natural)
		*natural = 0;
}

static void t_bare_container_realize(GtkWidget* widget)
{
	BareContainerPrivate* pr = T_BARE_CONTAINER_PRIVATE(widget);
	T_FATAL_ASSERT(pr != nullptr);

	gtk_widget_set_realized(widget, TRUE);

	GtkAllocation a;
	gtk_widget_get_allocation(widget, &a);

	GdkWindowAttr attr;
	attr.x = a.x;
	attr.y = a.y;
	attr.width = a.width;
	attr.height = a.height;
	attr.window_type = GDK_WINDOW_CHILD;
	attr.wclass = GDK_INPUT_OUTPUT;
	attr.event_mask = gtk_widget_get_events(widget); // | GDK_EXPOSURE_MASK;

	pr->window = gdk_window_new(
		gtk_widget_get_parent_window(widget),
		&attr,
		GDK_WA_X | GDK_WA_Y
	);
	gdk_window_set_user_data(pr->window, widget);
	gtk_widget_set_window(widget, pr->window);
}

static void t_bare_container_size_allocate(GtkWidget* widget, GtkAllocation* allocation)
{
	BareContainerPrivate* pr = T_BARE_CONTAINER_PRIVATE(widget);
	T_FATAL_ASSERT(pr != nullptr);

	gtk_widget_set_allocation(widget, allocation);

	if (gtk_widget_get_realized(widget))
	{
		gdk_window_move_resize(
			pr->window,
			allocation->x,
			allocation->y,
			allocation->width,
			allocation->height
		);
	}

	pr->ox = 0; // allocation->x;
	pr->oy = 0; //allocation->y;
}

static gboolean t_bare_container_draw(GtkWidget* widget, cairo_t* cr)
{
	return TRUE;
}

static GType t_bare_container_child_type(GtkContainer* container)
{
	return GTK_TYPE_WIDGET;
}

static void t_bare_container_add(GtkContainer* container, GtkWidget* widget)
{
	BareContainerPrivate* pr = T_BARE_CONTAINER_PRIVATE(container);
	T_FATAL_ASSERT(pr != nullptr);

	pr->children = g_list_append(pr->children, widget);
	gtk_widget_set_parent(widget, GTK_WIDGET(container));

	if (gtk_widget_get_visible(widget))
		gtk_widget_queue_resize(GTK_WIDGET(container));
}

static void t_bare_container_remove(GtkContainer* container, GtkWidget* widget)
{
	BareContainerPrivate* pr = T_BARE_CONTAINER_PRIVATE(container);
	T_FATAL_ASSERT(pr != nullptr);
	
	GList* link = g_list_find(pr->children, widget);	
	if (link)
	{
		gtk_widget_unparent(widget);
		pr->children = g_list_delete_link(pr->children, link);
	}
}

static void t_bare_container_forall(GtkContainer* container, gboolean includeInternals, GtkCallback callback, gpointer callbackData)
{
	BareContainerPrivate* pr = T_BARE_CONTAINER_PRIVATE(container);
	T_FATAL_ASSERT(pr != nullptr);

	g_list_foreach(pr->children, (GFunc)callback, callbackData);
}
