/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_animation_LwsEmpty_H
#define traktor_animation_LwsEmpty_H

#include "Animation/Editor/LwsParser/LwsNode.h"

namespace traktor
{
	namespace animation
	{

/*! \brief
 * \ingroup Animation
 */
class LwsEmpty : public LwsNode
{
	T_RTTI_CLASS;

public:
	LwsEmpty();
};

	}
}

#endif	// traktor_animation_LwsEmpty_H
