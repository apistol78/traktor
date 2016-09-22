#ifndef traktor_animation_LwsGroup_H
#define traktor_animation_LwsGroup_H

#include "Animation/Editor/LwsParser/LwsNode.h"
#include "Core/RefArray.h"

namespace traktor
{
	namespace animation
	{

/*! \brief
 * \ingroup Animation
 */
class LwsGroup : public LwsNode
{
	T_RTTI_CLASS;

public:
	LwsGroup(const std::wstring& name);

	void add(LwsNode* node);

	uint32_t getCount() const;

	Ref< const LwsNode > get(uint32_t index) const;

	Ref< const LwsNode > find(const std::wstring& name, uint32_t from = 0) const;

private:
	RefArray< LwsNode > m_nodes;
};

	}
}

#endif	// traktor_animation_LwsGroup_H
