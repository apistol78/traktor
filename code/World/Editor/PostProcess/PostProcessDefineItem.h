#ifndef traktor_world_PostProcessDefineItem_H
#define traktor_world_PostProcessDefineItem_H

#include "Core/Object.h"

namespace traktor
{
	namespace world
	{

class PostProcessDefine;

class PostProcessDefineItem : public Object
{
	T_RTTI_CLASS;

public:
	PostProcessDefineItem(PostProcessDefine* definition, const std::wstring& text, int32_t image);

	PostProcessDefine* getDefinition() const;

	const std::wstring& getText() const;

	int32_t getImage() const;

private:
	Ref< PostProcessDefine > m_definition;
	std::wstring m_text;
	int32_t m_image;
};

	}
}

#endif	// traktor_world_PostProcessDefineItem_H
