#ifndef traktor_flash_Tween_H
#define traktor_flash_Tween_H

#include "Flash/Action/ActionObjectRelay.h"

namespace traktor
{
	namespace flash
	{

class Tween : public ActionObjectRelay
{
	T_RTTI_CLASS;

public:
	Tween(ActionContext* context);

	void init(
		ActionObject* target,
		const std::string& propertyName,
		ActionFunction* function,
		avm_number_t begin,
		avm_number_t finish,
		avm_number_t duration,
		bool useSeconds
	);

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
	ActionContext* m_context;
	avm_number_t m_begin;
	avm_number_t m_finish;
	avm_number_t m_duration;
	bool m_useSeconds;
	avm_number_t m_timeStart;
	avm_number_t m_current;
	bool m_playing;

	void onFrame(CallArgs& ca);
};

	}
}

#endif	// traktor_flash_Tween_H
