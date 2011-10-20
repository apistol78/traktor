#include "Core/Io/StringOutputStream.h"
#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Avm1/Classes/As_flash_geom_Rectangle.h"
#include "Flash/Action/Classes/Point.h"
#include "Flash/Action/Classes/Rectangle.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.As_flash_geom_Rectangle", As_flash_geom_Rectangle, ActionClass)

As_flash_geom_Rectangle::As_flash_geom_Rectangle(ActionContext* context)
:	ActionClass(context, "flash.geom.Rectangle")
{
	Ref< ActionObject > prototype = new ActionObject(context);

	prototype->setMember("clone", ActionValue(createNativeFunction(context, this, &As_flash_geom_Rectangle::Rectangle_clone)));
	prototype->setMember("contains", ActionValue(createNativeFunction(context, this, &As_flash_geom_Rectangle::Rectangle_contains)));
	prototype->setMember("containsPoint", ActionValue(createNativeFunction(context, this, &As_flash_geom_Rectangle::Rectangle_containsPoint)));
	prototype->setMember("containsRectangle", ActionValue(createNativeFunction(context, this, &As_flash_geom_Rectangle::Rectangle_containsRectangle)));
	prototype->setMember("equals", ActionValue(createNativeFunction(context, this, &As_flash_geom_Rectangle::Rectangle_equals)));
	prototype->setMember("inflate", ActionValue(createNativeFunction(context, this, &As_flash_geom_Rectangle::Rectangle_inflate)));
	prototype->setMember("inflatePoint", ActionValue(createNativeFunction(context, this, &As_flash_geom_Rectangle::Rectangle_inflatePoint)));
	prototype->setMember("intersection", ActionValue(createNativeFunction(context, this, &As_flash_geom_Rectangle::Rectangle_intersection)));
	prototype->setMember("intersects", ActionValue(createNativeFunction(context, this, &As_flash_geom_Rectangle::Rectangle_intersects)));
	prototype->setMember("isEmpty", ActionValue(createNativeFunction(context, this, &As_flash_geom_Rectangle::Rectangle_isEmpty)));
	prototype->setMember("offset", ActionValue(createNativeFunction(context, this, &As_flash_geom_Rectangle::Rectangle_offset)));
	prototype->setMember("offsetPoint", ActionValue(createNativeFunction(context, this, &As_flash_geom_Rectangle::Rectangle_offsetPoint)));
	prototype->setMember("setEmpty", ActionValue(createNativeFunction(context, this, &As_flash_geom_Rectangle::Rectangle_setEmpty)));
	prototype->setMember("toString", ActionValue(createNativeFunction(context, this, &As_flash_geom_Rectangle::Rectangle_toString)));
	prototype->setMember("union", ActionValue(createNativeFunction(context, this, &As_flash_geom_Rectangle::Rectangle_union)));

	prototype->addProperty("bottom", createNativeFunction(context, this, &As_flash_geom_Rectangle::Rectangle_get_bottom), createNativeFunction(context, this, &As_flash_geom_Rectangle::Rectangle_set_bottom));
	prototype->addProperty("bottomRight", createNativeFunction(context, this, &As_flash_geom_Rectangle::Rectangle_get_bottomRight), createNativeFunction(context, this, &As_flash_geom_Rectangle::Rectangle_set_bottomRight));
	prototype->addProperty("height", createNativeFunction(context, this, &As_flash_geom_Rectangle::Rectangle_get_height), createNativeFunction(context, this, &As_flash_geom_Rectangle::Rectangle_set_height));
	prototype->addProperty("left", createNativeFunction(context, this, &As_flash_geom_Rectangle::Rectangle_get_left), createNativeFunction(context, this, &As_flash_geom_Rectangle::Rectangle_set_left));
	prototype->addProperty("right", createNativeFunction(context, this, &As_flash_geom_Rectangle::Rectangle_get_right), createNativeFunction(context, this, &As_flash_geom_Rectangle::Rectangle_set_right));
	prototype->addProperty("size", createNativeFunction(context, this, &As_flash_geom_Rectangle::Rectangle_get_size), createNativeFunction(context, this, &As_flash_geom_Rectangle::Rectangle_set_size));
	prototype->addProperty("top", createNativeFunction(context, this, &As_flash_geom_Rectangle::Rectangle_get_top), createNativeFunction(context, this, &As_flash_geom_Rectangle::Rectangle_set_top));
	prototype->addProperty("topLeft", createNativeFunction(context, this, &As_flash_geom_Rectangle::Rectangle_get_topLeft), createNativeFunction(context, this, &As_flash_geom_Rectangle::Rectangle_set_topLeft));
	prototype->addProperty("width", createNativeFunction(context, this, &As_flash_geom_Rectangle::Rectangle_get_width), createNativeFunction(context, this, &As_flash_geom_Rectangle::Rectangle_set_width));
	prototype->addProperty("x", createNativeFunction(context, this, &As_flash_geom_Rectangle::Rectangle_get_x), createNativeFunction(context, this, &As_flash_geom_Rectangle::Rectangle_set_x));
	prototype->addProperty("y", createNativeFunction(context, this, &As_flash_geom_Rectangle::Rectangle_get_y), createNativeFunction(context, this, &As_flash_geom_Rectangle::Rectangle_set_y));

	prototype->setMember("constructor", ActionValue(this));
	prototype->setMember("__coerce__", ActionValue(this));

	prototype->setReadOnly();

	setMember("prototype", ActionValue(prototype));
}

void As_flash_geom_Rectangle::init(ActionObject* self, const ActionValueArray& args)
{
	Ref< Rectangle > rc = new Rectangle();
	if (args.size() >= 4)
	{
		rc->left = args[0].getNumber();
		rc->top = args[1].getNumber();
		rc->width = args[2].getNumber();
		rc->height = args[3].getNumber();
	}
	self->setRelay(rc);
}

void As_flash_geom_Rectangle::coerce(ActionObject* self) const
{
	ActionValue left, top;
	ActionValue right, bottom;

	self->getMember("left", left);
	self->getMember("top", top);
	self->getMember("right", right);
	self->getMember("bottom", bottom);

	self->setRelay(new Rectangle(
		float(left.getNumber()),
		float(top.getNumber()),
		float(right.getNumber()),
		float(bottom.getNumber())
	));
}

Ref< Rectangle > As_flash_geom_Rectangle::Rectangle_clone(const Rectangle* self) const
{
	return new Rectangle(
		self->left,
		self->top,
		self->width,
		self->height
	);
}

bool As_flash_geom_Rectangle::Rectangle_contains(const Rectangle* self, avm_number_t x, avm_number_t y) const
{
	return self->contains(x, y);
}

bool As_flash_geom_Rectangle::Rectangle_containsPoint(const Rectangle* self, const Point* pt) const
{
	return self->contains(pt->x, pt->y);
}

bool As_flash_geom_Rectangle::Rectangle_containsRectangle(const Rectangle* self, const Rectangle* rc) const
{
	return self->contains(rc->left, rc->top) && self->contains(rc->left + rc->width, rc->top + rc->height);
}

bool As_flash_geom_Rectangle::Rectangle_equals(const Rectangle* self, const Rectangle* rc) const
{
	return 
		self->left == rc->left &&
		self->top == rc->top &&
		self->width == rc->width &&
		self->height == rc->height;
}

void As_flash_geom_Rectangle::Rectangle_inflate(Rectangle* self, avm_number_t dx, avm_number_t dy) const
{
	self->inflate(dx, dy);
}

void As_flash_geom_Rectangle::Rectangle_inflatePoint(Rectangle* self, const Point* pt) const
{
	self->inflate(pt->x, pt->y);
}

void As_flash_geom_Rectangle::Rectangle_intersection(const Rectangle* self) const
{
}

void As_flash_geom_Rectangle::Rectangle_intersects(const Rectangle* self) const
{
}

bool As_flash_geom_Rectangle::Rectangle_isEmpty(const Rectangle* self) const
{
	return self->isEmpty();
}

void As_flash_geom_Rectangle::Rectangle_offset(Rectangle* self, avm_number_t dx, avm_number_t dy) const
{
	self->offset(dx, dy);
}

void As_flash_geom_Rectangle::Rectangle_offsetPoint(Rectangle* self, const Point* pt) const
{
	self->offset(pt->x, pt->y);
}

void As_flash_geom_Rectangle::Rectangle_setEmpty(Rectangle* self) const
{
	self->left =
	self->top =
	self->width =
	self->height = avm_number_t(0);
}

ActionValue As_flash_geom_Rectangle::Rectangle_toString(const Rectangle* self) const
{
	StringOutputStream ss;
	ss << L"(x=" << self->left << L", y=" << self->top << L", w=" << self->width << L", h=" << self->height << L")";
	return ActionValue(ss.str());
}

void As_flash_geom_Rectangle::Rectangle_union(const Rectangle* self) const
{
}

avm_number_t As_flash_geom_Rectangle::Rectangle_get_bottom(const Rectangle* self) const
{
	return self->top + self->height;
}

void As_flash_geom_Rectangle::Rectangle_set_bottom(Rectangle* self, avm_number_t bottom) const
{
	self->height = bottom - self->top;
}

Ref< Point > As_flash_geom_Rectangle::Rectangle_get_bottomRight(const Rectangle* self) const
{
	return new Point(self->left + self->width, self->top + self->height);
}

void As_flash_geom_Rectangle::Rectangle_set_bottomRight(Rectangle* self, const Point* pt) const
{
	self->width = pt->x - self->left;
	self->height = pt->y - self->top;
}

avm_number_t As_flash_geom_Rectangle::Rectangle_get_height(const Rectangle* self) const
{
	return self->height;
}

void As_flash_geom_Rectangle::Rectangle_set_height(Rectangle* self, avm_number_t height) const
{
	self->height = height;
}

avm_number_t As_flash_geom_Rectangle::Rectangle_get_left(const Rectangle* self) const
{
	return self->left;
}

void As_flash_geom_Rectangle::Rectangle_set_left(Rectangle* self, avm_number_t left) const
{
	self->left = left;
}

avm_number_t As_flash_geom_Rectangle::Rectangle_get_right(const Rectangle* self) const
{
	return self->left + self->width;
}

void As_flash_geom_Rectangle::Rectangle_set_right(Rectangle* self, avm_number_t right) const
{
	self->width = right - self->left;
}

Ref< Point > As_flash_geom_Rectangle::Rectangle_get_size(const Rectangle* self) const
{
	return new Point(self->width, self->height);
}

void As_flash_geom_Rectangle::Rectangle_set_size(Rectangle* self, const Point* pt) const
{
	self->width = pt->x;
	self->height = pt->y;
}

avm_number_t As_flash_geom_Rectangle::Rectangle_get_top(const Rectangle* self) const
{
	return self->top;
}

void As_flash_geom_Rectangle::Rectangle_set_top(Rectangle* self, avm_number_t top) const
{
	self->top = top;
}

Ref< Point > As_flash_geom_Rectangle::Rectangle_get_topLeft(const Rectangle* self) const
{
	return new Point(self->left, self->top);
}

void As_flash_geom_Rectangle::Rectangle_set_topLeft(Rectangle* self, const Point* pt) const
{
	self->left = pt->x;
	self->top = pt->y;
}

avm_number_t As_flash_geom_Rectangle::Rectangle_get_width(const Rectangle* self) const
{
	return self->width;
}

void As_flash_geom_Rectangle::Rectangle_set_width(Rectangle* self, avm_number_t width) const
{
	self->width = width;
}

avm_number_t As_flash_geom_Rectangle::Rectangle_get_x(const Rectangle* self) const
{
	return self->left;
}

void As_flash_geom_Rectangle::Rectangle_set_x(Rectangle* self, avm_number_t x) const
{
	self->left = x;
}

avm_number_t As_flash_geom_Rectangle::Rectangle_get_y(const Rectangle* self) const
{
	return self->top;
}

void As_flash_geom_Rectangle::Rectangle_set_y(Rectangle* self, avm_number_t y) const
{
	self->top = y;
}

	}
}
