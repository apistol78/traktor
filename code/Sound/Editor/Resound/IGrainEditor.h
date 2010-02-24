#ifndef traktor_sound_IGrainEditor_H
#define traktor_sound_IGrainEditor_H

#include "Core/Object.h"

namespace traktor
{
	namespace ui
	{

class Widget;

	}

	namespace sound
	{

class IGrain;

class IGrainEditor : public Object
{
	T_RTTI_CLASS;

public:
	virtual bool create(ui::Widget* parent) = 0;

	virtual void destroy() = 0;

	virtual void show(IGrain* grain) = 0;

	virtual void hide() = 0;
};

	}
}

#endif	// traktor_sound_IGrainEditor_H
