#ifndef traktor_sound_GrainViewItem_H
#define traktor_sound_GrainViewItem_H

#include "Core/Object.h"
#include "Core/Ref.h"

namespace traktor
{
	namespace sound
	{

class IGrainData;

class GrainViewItem : public Object
{
	T_RTTI_CLASS;

public:
	GrainViewItem(GrainViewItem* parent, IGrainData* grain, const std::wstring& text, int32_t image);

	GrainViewItem* getParent() const;

	IGrainData* getGrain() const;

	int32_t getImage() const;

	std::wstring getText() const;

private:
	GrainViewItem* m_parent;
	Ref< IGrainData > m_grain;
	std::wstring m_text;
	int32_t m_image;
};

	}
}

#endif	// traktor_sound_GrainViewItem_H
