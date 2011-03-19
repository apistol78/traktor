#ifndef traktor_world_PostProcessStepItem_H
#define traktor_world_PostProcessStepItem_H

#include "Core/Object.h"

namespace traktor
{
	namespace world
	{

class PostProcessStep;

class PostProcessStepItem : public Object
{
	T_RTTI_CLASS;

public:
	PostProcessStepItem(PostProcessStepItem* parent, PostProcessStep* step, const std::wstring& text, int32_t image);

	PostProcessStepItem* getParent() const;

	PostProcessStep* getStep() const;

	const std::wstring& getText() const;

	int32_t getImage() const;

private:
	PostProcessStepItem* m_parent;
	Ref< PostProcessStep > m_step;
	std::wstring m_text;
	int32_t m_image;
};

	}
}

#endif	// traktor_world_PostProcessStepItem_H
