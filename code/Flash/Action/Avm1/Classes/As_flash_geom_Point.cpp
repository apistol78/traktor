#include <cmath>
#include <limits>
#include "Core/Io/StringOutputStream.h"
#include "Core/Math/MathUtils.h"
#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Avm1/Classes/As_flash_geom_Point.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.As_flash_geom_Point", As_flash_geom_Point, ActionClass)

As_flash_geom_Point::As_flash_geom_Point(ActionContext* context)
:	ActionClass(context, "flash.geom.Point")
{
	setMember("distance", ActionValue(createNativeFunction(context, this, &As_flash_geom_Point::Point_distance)));
	setMember("interpolate", ActionValue(createNativeFunction(context, this, &As_flash_geom_Point::Point_interpolate)));
	setMember("polar", ActionValue(createNativeFunction(context, this, &As_flash_geom_Point::Point_polar)));

	Ref< ActionObject > prototype = new ActionObject(context);

	prototype->setMember("add", ActionValue(createNativeFunction(context, this, &As_flash_geom_Point::Point_add)));
	prototype->setMember("clone", ActionValue(createNativeFunction(context, this, &As_flash_geom_Point::Point_clone)));
	prototype->setMember("equals", ActionValue(createNativeFunction(context, this, &As_flash_geom_Point::Point_equals)));
	prototype->setMember("normalize", ActionValue(createNativeFunction(context, this, &As_flash_geom_Point::Point_normalize)));
	prototype->setMember("offset", ActionValue(createNativeFunction(context, this, &As_flash_geom_Point::Point_offset)));
	prototype->setMember("subtract", ActionValue(createNativeFunction(context, this, &As_flash_geom_Point::Point_subtract)));
	prototype->setMember("toString", ActionValue(createNativeFunction(context, this, &As_flash_geom_Point::Point_toString)));

	prototype->addProperty("length", createNativeFunction(context, this, &As_flash_geom_Point::Point_get_length), createNativeFunction(context, this, &As_flash_geom_Point::Point_set_length));

	prototype->setMember("constructor", ActionValue(this));

	prototype->setReadOnly();

	setMember("prototype", ActionValue(prototype));
}

void As_flash_geom_Point::initialize(ActionObject* self)
{
}

void As_flash_geom_Point::construct(ActionObject* self, const ActionValueArray& args)
{
	if (args.size() >= 2)
	{
		self->setMember("x", args[0]);
		self->setMember("y", args[1]);
	}
	else
	{
		self->setMember("x", ActionValue(avm_number_t(0)));
		self->setMember("y", ActionValue(avm_number_t(0)));
	}
}

ActionValue As_flash_geom_Point::xplicit(const ActionValueArray& args)
{
	return ActionValue();
}

void As_flash_geom_Point::Point_distance(CallArgs& ca)
{
	if (ca.args.size() >= 2)
	{
		ActionObject* pt1 = ca.args[0].getObject();
		ActionObject* pt2 = ca.args[1].getObject();

		ActionValue x1, y1;
		pt1->getMember("x", x1);
		pt1->getMember("y", y1);

		ActionValue x2, y2;
		pt2->getMember("x", x2);
		pt2->getMember("y", y2);

		ActionValue dx = x2 - x1;
		ActionValue dy = y2 - y1;

		if (dx.isNumeric() && dy.isNumeric())
		{
			avm_number_t nx = dx.getNumber();
			avm_number_t ny = dy.getNumber();
			ca.ret = ActionValue(avm_number_t(sqrtf(nx * nx + ny * ny)));
		}
	}
}

void As_flash_geom_Point::Point_interpolate(CallArgs& ca)
{
	if (ca.args.size() >= 3)
	{
		ActionObject* pt1 = ca.args[0].getObject();
		ActionObject* pt2 = ca.args[1].getObject();
		avm_number_t f = ca.args[2].getNumber();

		if (pt1 && pt2)
		{
			ActionValue x1, y1;
			pt1->getMember("x", x1);
			pt1->getMember("y", y1);

			ActionValue x2, y2;
			pt2->getMember("x", x2);
			pt2->getMember("y", y2);

			Ref< ActionObject > pt = new ActionObject(getContext(), "flash.geom.Point");
			pt->setMember("x", x1 * ActionValue(1.0f - f) + x2 * ActionValue(f));
			pt->setMember("y", y1 * ActionValue(1.0f - f) + y2 * ActionValue(f));

			ca.ret = ActionValue(pt);
		}
	}
}

void As_flash_geom_Point::Point_polar(CallArgs& ca)
{
	if (ca.args.size() >= 2)
	{
		avm_number_t length = ca.args[0].getNumber();
		avm_number_t angle = ca.args[1].getNumber();

		Ref< ActionObject > pt = new ActionObject(getContext(), "flash.geom.Point");
		pt->setMember("x", ActionValue(cosf(angle) * length));
		pt->setMember("y", ActionValue(sinf(angle) * length));

		ca.ret = ActionValue(pt);
	}
}

void As_flash_geom_Point::Point_add(CallArgs& ca)
{
	Ref< ActionObject > pt = new ActionObject(getContext(), "flash.geom.Point");

	ActionValue x1, y1;
	ca.self->getMember("x", x1);
	ca.self->getMember("y", y1);

	ActionValue x2, y2;
	if (ca.args.size() >= 1)
	{
		ActionObject* pt2 = ca.args[0].getObject();
		if (pt2)
		{
			pt2->getMember("x", x2);
			pt2->getMember("y", y2);
		}
		else if (ca.args.size() >= 2)
		{
			x2 = ca.args[0];
			y2 = ca.args[1];
		}
	}

	pt->setMember("x", x1 + x2);
	pt->setMember("y", y1 + y2);

	ca.ret = ActionValue(pt);
}

void As_flash_geom_Point::Point_clone(CallArgs& ca)
{
	Ref< ActionObject > pt = new ActionObject(getContext(), "flash.geom.Point");
	
	ActionValue x, y;
	ca.self->getMember("x", x);
	ca.self->getMember("y", y);

	pt->setMember("x", x);
	pt->setMember("y", y);

	ca.ret = ActionValue(pt);
}

void As_flash_geom_Point::Point_equals(CallArgs& ca)
{
	bool equal = false;
	if (ca.args.size() >= 1)
	{
		ActionObject* pt = ca.args[0].getObject();
		if (pt)
		{
			ActionValue x1, y1;
			ca.self->getMember("x", x1);
			ca.self->getMember("y", y1);

			ActionValue x2, y2;
			pt->getMember("x", x2);
			pt->getMember("y", y2);

			equal = (x1 == x2) && (y1 == y2);
		}
	}
	ca.ret = ActionValue(equal);
}

void As_flash_geom_Point::Point_normalize(CallArgs& ca)
{
	avm_number_t scale = avm_number_t(1);

	if (ca.args.size() >= 1)
		scale = ca.args[0].getNumber();

	ActionValue x, y;
	ca.self->getMember("x", x);
	ca.self->getMember("y", y);

	if (x.isNumeric() && y.isNumeric())
	{
		avm_number_t nx = x.getNumber();
		avm_number_t ny = y.getNumber();

		avm_number_t ln = avm_number_t(sqrtf(nx * nx + ny * ny));
		if (ln > avm_number_t(0))
		{
			ca.self->setMember("x", x * ActionValue(scale / ln));
			ca.self->setMember("y", y * ActionValue(scale / ln));
		}
	}
}

void As_flash_geom_Point::Point_offset(CallArgs& ca)
{
	if (ca.args.size() >= 2)
	{
		const ActionValue& dx = ca.args[0];
		const ActionValue& dy = ca.args[1];

		ActionValue x, y;
		ca.self->getMember("x", x);
		ca.self->getMember("y", y);

		ca.self->setMember("x", x + dx);
		ca.self->setMember("y", y + dy);
	}
}

void As_flash_geom_Point::Point_subtract(CallArgs& ca)
{
	if (ca.args.size() >= 2)
	{
		ActionObject* pr = ca.args[0].getObject();
		if (!pr)
			return;

		ActionValue rx, ry;
		pr->getMember("x", rx);
		pr->getMember("y", ry);

		ActionValue x, y;
		ca.self->getMember("x", x);
		ca.self->getMember("y", y);

		ca.self->setMember("x", x - rx);
		ca.self->setMember("y", y - ry);
	}
}

void As_flash_geom_Point::Point_toString(CallArgs& ca)
{
	ActionValue x, y;
	ca.self->getMember("x", x);
	ca.self->getMember("y", y);

	StringOutputStream ss;
	ss << L"(x=" << x.getWideString() << L", y=" << y.getWideString() << L")";

	ca.ret = ActionValue(ss.str());
}

void As_flash_geom_Point::Point_get_length(CallArgs& ca)
{
	ActionValue x, y;
	ca.self->getMember("x", x);
	ca.self->getMember("y", y);

	if (x.isNumeric() && y.isNumeric())
	{
		avm_number_t nx = x.getNumber();
		avm_number_t ny = y.getNumber();
		avm_number_t ln = avm_number_t(sqrtf(nx * nx + ny * ny));
		ca.ret = ActionValue(ln);
	}
	else
		ca.ret = ActionValue(avm_number_t(
			std::numeric_limits< avm_number_t >::signaling_NaN()
		));
}

void As_flash_geom_Point::Point_set_length(CallArgs& ca)
{
}

	}
}
