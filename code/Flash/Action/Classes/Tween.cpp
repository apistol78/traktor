#include "Core/Log/Log.h"
#include "Core/Math/MathUtils.h"
#include "Flash/Action/ActionContext.h"
#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Classes/Tween.h"
#include "Flash/Action/Avm1/Classes/AsTween.h"

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
:	ActionObject(AsTween::getInstance())
,	m_context(context)
,	m_target(target)
,	m_function(function)
,	m_begin(begin)
,	m_finish(finish)
,	m_duration(duration)
,	m_useSeconds(useSeconds)
,	m_timeStart(avm_number_t(-1))
,	m_current(begin)
,	m_playing(false)
{
	if (m_target)
		m_target->getPropertySet(propertyName, m_property);
	
	setMember(L"onFrame", createNativeFunctionValue(this, &Tween::onFrame));
	start();
}

Tween::~Tween()
{
	stop();
}

void Tween::continueTo(avm_number_t finish, avm_number_t duration)
{
	m_begin = m_current;
	m_finish = finish;
	m_duration = duration;
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
	if (!m_playing)
		m_context->addFrameListener(this);

	m_timeStart = avm_number_t(-1);
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
	if (!m_function || !m_property || !m_playing)
		return;

	avm_number_t time = ca.args[0].getNumberSafe();

	if (m_timeStart < 0)
		m_timeStart = time;

	// Calculate interpolated value.
	avm_number_t t = time - m_timeStart;
	avm_number_t T = clamp(t, avm_number_t(0), m_duration);
	avm_number_t b = T / m_duration;

	// Calculate eased value.
	std::vector< ActionValue > argv0(4);
	argv0[0] = ActionValue(T);
	argv0[1] = ActionValue(m_begin);
	argv0[2] = ActionValue(m_finish - m_begin);
	argv0[3] = ActionValue(m_duration);

	ActionValue value = m_function->call(ca.vm, ca.context, this, argv0);
	m_current = value.getNumberSafe();

	// Set property value.
	std::vector< ActionValue > argv1(1);
	argv1[0] = value;
	m_property->call(ca.vm, ca.context, m_target, argv1);

	// Stop after duration expired.
	if (m_playing && t >= m_duration)
	{
		stop();

		// Notify listener when we've reached the end.
		ActionValue memberValue;
		if (getLocalMember(L"onMotionFinished", memberValue))
		{
			Ref< ActionFunction > motionFinished = memberValue.getObject< ActionFunction >();
			if (motionFinished)
				motionFinished->call(ca.vm, ca.context, this, std::vector< ActionValue >());
		}
	}
}

	}
}
