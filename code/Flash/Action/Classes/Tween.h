#ifndef traktor_flash_Tween_H
#define traktor_flash_Tween_H

#include "Flash/Action/ActionObject.h"

namespace traktor
{
	namespace flash
	{

class ActionContext;
class IActionVM;

class Tween : public ActionObject
{
	T_RTTI_CLASS;

public:
	Tween(
		ActionContext* context,
		ActionObject* target,
		const std::wstring& propertyName,
		ActionFunction* function,
		avm_number_t begin,
		avm_number_t finish,
		avm_number_t duration,
		bool useSeconds
	);

	virtual ~Tween();

	void continueTo(avm_number_t finish, avm_number_t duration);

	void fforward();

	void nextFrame();

	void prevFrame();

	void resume();

	void rewind(avm_number_t t);

	void start();

	void stop();

	void yoyo();

private:
	Ref< ActionContext > m_context;
	Ref< ActionFunction > m_frameListener;
	Ref< ActionObject > m_target;
	Ref< ActionFunction > m_property;
	Ref< ActionFunction > m_function;
	avm_number_t m_begin;
	avm_number_t m_finish;
	avm_number_t m_duration;
	bool m_useSeconds;

	avm_number_t m_timeStart;
	bool m_playing;

	void onFrame(CallArgs& ca);
};

	}
}

#endif	// traktor_flash_Tween_H
