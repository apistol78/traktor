#ifndef traktor_sound_GrainViewItem_H
#define traktor_sound_GrainViewItem_H

#include "Core/Object.h"
#include "Core/Ref.h"

namespace traktor
{
	namespace sound
	{

class IGrain;

class GrainViewItem : public Object
{
	T_RTTI_CLASS;

public:
	GrainViewItem(GrainViewItem* parent, IGrain* grain, const std::wstring& text, int32_t image);

	GrainViewItem* getParent() const;

	IGrain* getGrain() const;

	int32_t getImage() const;

	std::wstring getText() const;

private:
	GrainViewItem* m_parent;
	Ref< IGrain > m_grain;
	std::wstring m_text;
	int32_t m_image;
};

	}
}

#endif	// traktor_sound_GrainViewItem_H
