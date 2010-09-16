#include "Core/Log/Log.h"
#include "Core/Math/MathUtils.h"
#include "Flash/Action/ActionContext.h"
#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Classes/Tween.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.Tween", Tween, ActionObject)

Tween::Tween(
	ActionContext* context,
	ActionObject* target,
	const std::wstring& propertyName,
	ActionFunction* function,
	avm_number_t begin,
	avm_number_t finish,
	avm_number_t duration,
	bool useSeconds
)
:	ActionObject(L"mx.transitions.Tween")
,	m_context(context)
,	m_begin(begin)
,	m_finish(finish)
,	m_duration(duration)
,	m_useSeconds(useSeconds)
,	m_timeStart(avm_number_t(-1))
,	m_current(begin)
,	m_playing(false)
{
	if (target)
	{
		setMember(L"_target", ActionValue(target));

		Ref< ActionFunction > propertySet;
		if (target->getPropertySet(context, propertyName, propertySet))
			setMember(L"_targetProperty", ActionValue(propertySet));
	}
	
	if (function)
		setMember(L"_function", ActionValue(function));

	setMember(L"onFrame", ActionValue(createNativeFunction(this, &Tween::onFrame)));
	start();
}

void Tween::continueTo(avm_number_t finish, avm_number_t duration)
{
	m_begin = m_current;
	m_finish = finish;
	m_duration = duration;
	m_timeStart = avm_number_t(-1);
	resume();
}

void Tween::fforward()
{
}

void Tween::nextFrame()
{
}

void Tween::prevFrame()
{
}

void Tween::resume()
{
	if (!m_playing)
		m_context->addFrameListener(this);

	m_playing = true;
}

void Tween::rewind(avm_number_t t)
{
	m_timeStart = avm_number_t(-1);
}

void Tween::start()
{
	m_timeStart = avm_number_t(-1);

	if (!m_playing)
	{
		ActionValue target;
		ActionValue propertySet;
		ActionValue function;

		getLocalMember(L"_target", target);
		getLocalMember(L"_targetProperty", propertySet);
		getLocalMember(L"_function", function);

		// Ensure property is set to initial value.
		if (propertySet.isObject() && function.isObject())
		{
			ActionValue value;

			// Calculate eased value.
			if (m_duration > 0.0f)
			{
				ActionValueArray argv0(m_context->getPool(), 4);
				argv0[0] = ActionValue(avm_number_t(0));
				argv0[1] = ActionValue(m_begin);
				argv0[2] = ActionValue(m_finish - m_begin);
				argv0[3] = ActionValue(m_duration);
				value = function.getObject< ActionFunction >()->call(m_context, this, argv0);
			}
			else
				value = ActionValue(m_begin);

			m_current = value.getNumberSafe();

			// Set property value.
			if (!value.isUndefined())
			{
				ActionValueArray argv1(m_context->getPool(), 1);
				argv1[0] = value;
				propertySet.getObject< ActionFunction >()->call(
					m_context,
					target.getObjectSafe(),
					argv1
				);
			}
		}

		m_context->addFrameListener(this);
	}

	m_playing = true;
}

void Tween::stop()
{
	if (m_playing)
		m_context->removeFrameListener(this);

	m_playing = false;
}

void Tween::yoyo()
{
	std::swap(m_begin, m_finish);
	start();
}

void Tween::onFrame(CallArgs& ca)
{
	ActionValue target;
	ActionValue propertySet;
	ActionValue function;

	getLocalMember(L"_target", target);
	getLocalMember(L"_targetProperty", propertySet);
	getLocalMember(L"_function", function);

	if (!function.isObject() || !propertySet.isObject() || !m_playing || m_duration <= 0.0f)
		return;

	ActionValue value;

	avm_number_t time = ca.args[0].getNumberSafe();
	if (m_timeStart < 0)
		m_timeStart = time;

	// Calculate interpolated value.
	avm_number_t t = time - m_timeStart;
	avm_number_t T = clamp(t, avm_number_t(0), m_duration);

	// Calculate eased value.
	if (m_duration > 0.0f)
	{
		ActionValueArray argv0(ca.context->getPool(), 4);
		argv0[0] = ActionValue(T);
		argv0[1] = ActionValue(m_begin);
		argv0[2] = ActionValue(m_finish - m_begin);
		argv0[3] = ActionValue(m_duration);
		value = function.getObject< ActionFunction >()->call(ca.context, this, argv0);
	}
	else
		value = ActionValue(m_begin);

	m_current = value.getNumberSafe();

	// Set property value.
	if (!value.isUndefined())
	{
		ActionValueArray argv1(ca.context->getPool(), 1);
		argv1[0] = value;
		propertySet.getObject< ActionFunction >()->call(
			ca.context,
			target.getObjectSafe(),
			argv1
		);
	}

	// Stop after duration expired.
	if (t >= m_duration)
	{
		stop();

		// Notify listener when we've reached the end.
		ActionValue memberValue;
		if (getLocalMember(L"onMotionFinished", memberValue))
		{
			Ref< ActionFunction > motionFinished = memberValue.getObjectSafe< ActionFunction >();
			if (motionFinished)
				motionFinished->call(ca.context, this, ActionValueArray());
		}
	}
}

	}
}
