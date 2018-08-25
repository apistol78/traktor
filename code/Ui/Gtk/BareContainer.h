#pragma once

#include <glib-object.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define T_BARE_CONTAINER_TYPE				(t_bare_container_get_type())
#define T_BARE_CONTAINER(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), T_BARE_CONTAINER_TYPE, BareContainer))
#define T_BARE_CONTAINER_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), T_BARE_CONTAINER_TYPE, BareContainerClass))
#define T_IS_BARE_CONTAINER(obj)			(G_TYPE_CHECK_INSTANCE_TYPE((obj), T_BARE_CONTAINER_TYPE))
#define T_IS_BARE_CONTAINER_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), T_BARE_CONTAINER_TYPE))

typedef struct _BareContainer BareContainer;
typedef struct _BareContainerClass BareContainerClass;

struct _BareContainer
{
	GtkContainer parent_instance;
};

struct _BareContainerClass
{
	GtkContainerClass parent_class;
};

GType t_bare_container_get_type() G_GNUC_CONST;

GtkWidget* t_bare_container_new();

void t_base_container_place_child(BareContainer* container, GtkWidget* child, int x, int y, int width, int height);

void t_base_container_get_child_position(BareContainer* container, GtkWidget* child, int* x, int* y);

G_END_DECLS
