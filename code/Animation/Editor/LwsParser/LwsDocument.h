/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_animation_LwsDocument_H
#define traktor_animation_LwsDocument_H

#include "Core/Object.h"

namespace traktor
{

class IStream;

	namespace animation
	{

class LwsGroup;

/*! \brief
 * \ingroup Animation
 */
class LwsDocument : public Object
{
	T_RTTI_CLASS;

public:
	static Ref< LwsDocument > parse(IStream* stream);

	Ref< LwsGroup > getRootGroup();

private:
	Ref< LwsGroup > m_rootGroup;
};

	}
}

#endif	// traktor_animation_LwsDocument_H
