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
		float begin,
		float finish,
		float duration,
		bool useSeconds
	);

	void continueTo(float finish, float duration);

	void fforward();

	void nextFrame();

	void prevFrame();

	void resume();

	void rewind(float t);

	void start();

	void stop();

	void yoyo();

private:
	ActionContext* m_context;
	float m_begin;
	float m_finish;
	float m_duration;
	bool m_useSeconds;
	float m_timeStart;
	float m_current;
	bool m_playing;

	void onFrame(CallArgs& ca);
};

	}
}

#endif	// traktor_flash_Tween_H
