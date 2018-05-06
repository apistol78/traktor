/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_animation_BvhDocument_H
#define traktor_animation_BvhDocument_H

#include <list>
#include <vector>
#include "Core/Object.h"
#include "Core/Ref.h"

namespace traktor
{

class IStream;
class Vector4;

	namespace animation
	{

class BvhJoint;

/*! \brief
 * \ingroup Animation
 */
class BvhDocument : public Object
{
	T_RTTI_CLASS;

public:
	typedef std::vector< float > cv_t;
	typedef std::list< cv_t > cv_list_t;

	static Ref< BvhDocument > parse(IStream* stream, const Vector4& jointModifier);

	BvhJoint* getRootJoint() const;

	float getFrameTime() const;

	const cv_list_t& getChannelValues() const;

private:
	Ref< BvhJoint > m_rootJoint;
	float m_frameTime;
	cv_list_t m_channelValues;
};

	}
}

#endif	// traktor_animation_BvhDocument_H
