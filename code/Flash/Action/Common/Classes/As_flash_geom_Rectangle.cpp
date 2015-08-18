#include "Core/Io/StringOutputStream.h"
#include "Flash/Action/ActionContext.h"
#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Common/Classes/As_flash_geom_Rectangle.h"

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
	prototype->addProperty("left", createNativeFunction(context, this, &As_flash_geom_Rectangle::Rectangle_get_left), createNativeFunction(context, this, &As_flash_geom_Rectangle::Rectangle_set_left));
	prototype->addProperty("right", createNativeFunction(context, this, &As_flash_geom_Rectangle::Rectangle_get_right), createNativeFunction(context, this, &As_flash_geom_Rectangle::Rectangle_set_right));
	prototype->addProperty("size", createNativeFunction(context, this, &As_flash_geom_Rectangle::Rectangle_get_size), createNativeFunction(context, this, &As_flash_geom_Rectangle::Rectangle_set_size));
	prototype->addProperty("top", createNativeFunction(context, this, &As_flash_geom_Rectangle::Rectangle_get_top), createNativeFunction(context, this, &As_flash_geom_Rectangle::Rectangle_set_top));
	prototype->addProperty("topLeft", createNativeFunction(context, this, &As_flash_geom_Rectangle::Rectangle_get_topLeft), createNativeFunction(context, this, &As_flash_geom_Rectangle::Rectangle_set_topLeft));

	prototype->setMember("constructor", ActionValue(this));

	prototype->setReadOnly();

	setMember("prototype", ActionValue(prototype));

	m_idX = context->getString("x");
	m_idY = context->getString("y");
	m_idWidth = context->getString("width");
	m_idHeight = context->getString("height");
}

void As_flash_geom_Rectangle::initialize(ActionObject* self)
{
}

void As_flash_geom_Rectangle::construct(ActionObject* self, const ActionValueArray& args)
{
	if (args.size() > 0)
	{
		self->setMember(m_idX, args[0]);
		
		if (args.size() >= 2)
			self->setMember(m_idY, args[1]);
		else
			self->setMember(m_idY, ActionValue());

		if (args.size() >= 3)
			self->setMember(m_idWidth, args[2]);
		else
			self->setMember(m_idWidth, ActionValue());

		if (args.size() >= 4)
			self->setMember(m_idHeight, args[3]);
		else
			self->setMember(m_idHeight, ActionValue());
	}
	else
	{
		self->setMember(m_idX, ActionValue(avm_number_t(0)));
		self->setMember(m_idY, ActionValue(avm_number_t(0)));
		self->setMember(m_idWidth, ActionValue(avm_number_t(0)));
		self->setMember(m_idHeight, ActionValue(avm_number_t(0)));
	}
}

ActionValue As_flash_geom_Rectangle::xplicit(const ActionValueArray& args)
{
	return ActionValue();
}

void As_flash_geom_Rectangle::Rectangle_clone(CallArgs& ca)
{
	Ref< ActionObject > rc = new ActionObject(getContext(), "flash.geom.Rectangle");

	ActionValue x, y, width, height;
	ca.self->getMember(m_idX, x);
	ca.self->getMember(m_idY, y);
	ca.self->getMember(m_idWidth, width);
	ca.self->getMember(m_idHeight, height);

	rc->setMember(m_idX, x);
	rc->setMember(m_idY, y);
	rc->setMember(m_idWidth, width);
	rc->setMember(m_idHeight, height);

	ca.ret = ActionValue(rc);
}

void As_flash_geom_Rectangle::Rectangle_contains(CallArgs& ca)
{
	avm_number_t rc[4];
	getRectangle(ca.self, rc);

	if (ca.args.size() >= 2)
	{
		avm_number_t x = ca.args[0].getNumber();
		avm_number_t y = ca.args[1].getNumber();

		ca.ret = ActionValue(
			(x >= rc[0] && y >= rc[1] && x <= rc[0] + rc[2] && y <= rc[1] + rc[3])
		);
		return;
	}

	ca.ret = ActionValue(false);
}

void As_flash_geom_Rectangle::Rectangle_containsPoint(CallArgs& ca)
{
	avm_number_t rc[4];
	getRectangle(ca.self, rc);

	if (ca.args.size() >= 1)
	{
		ActionObject* pt = ca.args[0].getObject();
		if (pt)
		{
			ActionValue x, y;
			pt->getMember(m_idX, x);
			pt->getMember(m_idY, y);

			if (x.isNumeric() && y.isNumeric())
			{
				avm_number_t px = x.getNumber();
				avm_number_t py = y.getNumber();
				ca.ret = ActionValue(
					(px >= rc[0] && py >= rc[1] && px <= rc[0] + rc[2] && py <= rc[1] + rc[3])
				);
				return;
			}
		}
	}

	ca.ret = ActionValue(false);
}

void As_flash_geom_Rectangle::Rectangle_containsRectangle(CallArgs& ca)
{
	T_IF_VERBOSE(
		log::warning << L"Rectangle::containsRectangle not implemented" << Endl;
	)
}

void As_flash_geom_Rectangle::Rectangle_equals(CallArgs& ca)
{
	bool equal = false;
	if (ca.args.size() >= 1)
	{
		ActionValue rc1[4], rc2[4];
		getRectangleValues(ca.self, rc1);
		getRectangleValues(ca.args[0].getObject(), rc2);

		equal = (
			rc1[0] == rc2[0] &&
			rc1[1] == rc2[1] &&
			rc1[2] == rc2[2] &&
			rc1[3] == rc2[3]
		);
	}
	ca.ret = ActionValue(equal);
}

void As_flash_geom_Rectangle::Rectangle_inflate(CallArgs& ca)
{
	ActionValue rc[4];
	getRectangleValues(ca.self, rc);

	if (ca.args.size() >= 2)
	{
		ActionValue x = ca.args[0];
		ActionValue y = ca.args[1];

		rc[0] = rc[0] - x;
		rc[1] = rc[1] - y;
		rc[2] = rc[2] + x * ActionValue(avm_number_t(2));
		rc[3] = rc[3] + y * ActionValue(avm_number_t(2));

		setRectangleValues(ca.self, rc);
	}
}

void As_flash_geom_Rectangle::Rectangle_inflatePoint(CallArgs& ca)
{
	ActionValue rc[4];
	getRectangleValues(ca.self, rc);

	if (ca.args.size() >= 1)
	{
		ActionObject* pt = ca.args[0].getObject();
		if (pt)
		{
			ActionValue x, y;
			pt->getMember(m_idX, x);
			pt->getMember(m_idY, y);

			rc[0] = rc[0] - x;
			rc[1] = rc[1] - y;
			rc[2] = rc[2] + x * ActionValue(avm_number_t(2));
			rc[3] = rc[3] + y * ActionValue(avm_number_t(2));

			setRectangleValues(ca.self, rc);
		}
	}
}

void As_flash_geom_Rectangle::Rectangle_intersection(CallArgs& ca)
{
	T_IF_VERBOSE(
		log::warning << L"Rectangle::intersection not implemented" << Endl;
	)
}

void As_flash_geom_Rectangle::Rectangle_intersects(CallArgs& ca)
{
	T_IF_VERBOSE(
		log::warning << L"Rectangle::intersects not implemented" << Endl;
	)
}

void As_flash_geom_Rectangle::Rectangle_isEmpty(CallArgs& ca)
{
	bool empty = true;

	avm_number_t sz[2];
	if (getSize(ca.self, sz))
		empty = bool(sz[0] <= avm_number_t(0) && sz[1] <= avm_number_t(0));

	ca.ret = ActionValue(empty);
}

void As_flash_geom_Rectangle::Rectangle_offset(CallArgs& ca)
{
	ActionValue rc[4];
	getRectangleValues(ca.self, rc);

	if (ca.args.size() >= 2)
	{
		ActionValue x = ca.args[0];
		ActionValue y = ca.args[1];

		rc[0] = rc[0] + x;
		rc[1] = rc[1] + y;

		setRectangleValues(ca.self, rc);
	}
}

void As_flash_geom_Rectangle::Rectangle_offsetPoint(CallArgs& ca)
{
	ActionValue rc[4];
	getRectangleValues(ca.self, rc);

	if (ca.args.size() >= 1)
	{
		ActionObject* pt = ca.args[0].getObject();
		if (pt)
		{
			ActionValue x, y;
			pt->getMember(m_idX, x);
			pt->getMember(m_idY, y);

			rc[0] = rc[0] + x;
			rc[1] = rc[1] + y;

			setRectangleValues(ca.self, rc);
		}
	}
}

void As_flash_geom_Rectangle::Rectangle_setEmpty(CallArgs& ca)
{
	ca.self->setMember(m_idX, ActionValue(avm_number_t(0)));
	ca.self->setMember(m_idY, ActionValue(avm_number_t(0)));
	ca.self->setMember(m_idWidth, ActionValue(avm_number_t(0)));
	ca.self->setMember(m_idHeight, ActionValue(avm_number_t(0)));
}

void As_flash_geom_Rectangle::Rectangle_toString(CallArgs& ca)
{
	ActionValue rc[4];
	getRectangleValues(ca.self, rc);

	StringOutputStream ss;
	ss << L"(x=" << rc[0].getWideString() << L", y=" << rc[1].getWideString() << L", w=" << rc[2].getWideString() << L", h=" << rc[3].getWideString() << L")";

	ca.ret = ActionValue(ss.str());
}

void As_flash_geom_Rectangle::Rectangle_union(CallArgs& ca)
{
	T_IF_VERBOSE(
		log::warning << L"Rectangle::union not implemented" << Endl;
	)
}

void As_flash_geom_Rectangle::Rectangle_get_bottom(CallArgs& ca)
{
	ActionValue rc[4];
	getRectangleValues(ca.self, rc);
	ca.ret = rc[1] + rc[3];
}

void As_flash_geom_Rectangle::Rectangle_set_bottom(CallArgs& ca)
{
	ActionValue rc[4];
	getRectangleValues(ca.self, rc);
	rc[3] = ca.args[0] - rc[1];
	setRectangleValues(ca.self, rc);
}

void As_flash_geom_Rectangle::Rectangle_get_bottomRight(CallArgs& ca)
{
	ActionValue rc[4];
	getRectangleValues(ca.self, rc);

	Ref< ActionObject > pt = new ActionObject(getContext(), "flash.geom.Point");
	pt->setMember(m_idX, rc[0] + rc[2]);
	pt->setMember(m_idY, rc[1] + rc[3]);

	ca.ret = ActionValue(pt);
}

void As_flash_geom_Rectangle::Rectangle_set_bottomRight(CallArgs& ca)
{
	ActionValue rc[4];
	getRectangleValues(ca.self, rc);

	ActionObject* pt = ca.args[0].getObject();
	if (pt)
	{
		ActionValue x, y;
		pt->getMember(m_idX, x);
		pt->getMember(m_idY, y);

		rc[2] = x - rc[0];
		rc[3] = y - rc[1];
	}
	
	setRectangleValues(ca.self, rc);
}

void As_flash_geom_Rectangle::Rectangle_get_left(CallArgs& ca)
{
	ca.self->getMember(m_idX, ca.ret);
}

void As_flash_geom_Rectangle::Rectangle_set_left(CallArgs& ca)
{
	ca.self->setMember(m_idX, ca.args[0]);
}

void As_flash_geom_Rectangle::Rectangle_get_right(CallArgs& ca)
{
	ActionValue rc[4];
	getRectangleValues(ca.self, rc);
	ca.ret = rc[0] + rc[2];
}

void As_flash_geom_Rectangle::Rectangle_set_right(CallArgs& ca)
{
	ActionValue rc[4];
	getRectangleValues(ca.self, rc);
	rc[2] = ca.args[0] - rc[0];
	setRectangleValues(ca.self, rc);
}

void As_flash_geom_Rectangle::Rectangle_get_size(CallArgs& ca)
{
	ActionValue rc[4];
	getRectangleValues(ca.self, rc);

	Ref< ActionObject > pt = new ActionObject(getContext(), "flash.geom.Point");
	pt->setMember(m_idX, rc[2]);
	pt->setMember(m_idY, rc[3]);

	ca.ret = ActionValue(pt);
}

void As_flash_geom_Rectangle::Rectangle_set_size(CallArgs& ca)
{
	ActionValue rc[4];
	getRectangleValues(ca.self, rc);

	ActionObject* pt = ca.args[0].getObject();
	if (pt)
	{
		ActionValue x, y;
		pt->getMember(m_idX, x);
		pt->getMember(m_idY, y);

		rc[2] = x;
		rc[3] = y;
	}

	setRectangleValues(ca.self, rc);
}

void As_flash_geom_Rectangle::Rectangle_get_top(CallArgs& ca)
{
	ca.self->getMember(m_idY, ca.ret);
}

void As_flash_geom_Rectangle::Rectangle_set_top(CallArgs& ca)
{
	ca.self->setMember(m_idY, ca.args[0]);
}

void As_flash_geom_Rectangle::Rectangle_get_topLeft(CallArgs& ca)
{
	ActionValue rc[4];
	getRectangleValues(ca.self, rc);

	Ref< ActionObject > pt = new ActionObject(getContext(), "flash.geom.Point");
	pt->setMember(m_idX, rc[0]);
	pt->setMember(m_idY, rc[1]);

	ca.ret = ActionValue(pt);
}

void As_flash_geom_Rectangle::Rectangle_set_topLeft(CallArgs& ca)
{
	ActionValue rc[4];
	getRectangleValues(ca.self, rc);

	ActionObject* pt = ca.args[0].getObject();
	if (pt)
	{
		ActionValue x, y;
		pt->getMember(m_idX, x);
		pt->getMember(m_idY, y);

		rc[0] = x;
		rc[1] = y;
	}

	setRectangleValues(ca.self, rc);
}

void As_flash_geom_Rectangle::getRectangleValues(ActionObject* self, ActionValue rc[4])
{
	if (self)
	{
		self->getMember(m_idX, rc[0]);
		self->getMember(m_idY, rc[1]);
		self->getMember(m_idWidth, rc[2]);
		self->getMember(m_idHeight, rc[3]);
	}
}

void As_flash_geom_Rectangle::setRectangleValues(ActionObject* self, ActionValue rc[4])
{
	if (self)
	{
		self->setMember(m_idX, rc[0]);
		self->setMember(m_idY, rc[1]);
		self->setMember(m_idWidth, rc[2]);
		self->setMember(m_idHeight, rc[3]);
	}
}

bool As_flash_geom_Rectangle::getSize(ActionObject* self, avm_number_t sz[2])
{
	ActionValue szv[2];
	self->getMember(m_idWidth, szv[0]);
	self->getMember(m_idHeight, szv[1]);

	if (szv[0].isNumeric() && szv[1].isNumeric())
	{
		sz[0] = szv[0].getNumber();
		sz[1] = szv[1].getNumber();
		return true;
	}

	return false;
}

bool As_flash_geom_Rectangle::getRectangle(ActionObject* self, avm_number_t rc[4])
{
	ActionValue rcv[4];
	getRectangleValues(self, rcv);

	for (int i = 0; i < 4; ++i)
	{
		if (!rcv[i].isNumeric())
			return false;

		rc[i] = rcv[i].getNumber();
	}

	return true;
}

void As_flash_geom_Rectangle::setRectangle(ActionObject* self, avm_number_t rc[4])
{
	if (self)
	{
		self->setMember(m_idX, ActionValue(rc[0]));
		self->setMember(m_idY, ActionValue(rc[1]));
		self->setMember(m_idWidth, ActionValue(rc[2]));
		self->setMember(m_idHeight, ActionValue(rc[3]));
	}
}

	}
}
