/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_spray_Effect_H
#define traktor_spray_Effect_H

#include "Core/RefArray.h"
#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPRAY_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace spray
	{

class EffectInstance;
class EffectLayer;

/*! \brief
 * \ingroup Spray
 */
class T_DLLCLASS Effect : public Object
{
	T_RTTI_CLASS;

public:
	Effect(
		float duration,
		float loopStart,
		float loopEnd,
		const RefArray< EffectLayer >& layers
	);

	Ref< EffectInstance > createInstance() const;

	float getDuration() const { return m_duration; }

	float getLoopStart() const { return m_loopStart; }

	float getLoopEnd() const { return m_loopEnd; }

	const RefArray< EffectLayer >& getLayers() const { return m_layers; }
	
private:
	float m_duration;
	float m_loopStart;
	float m_loopEnd;
	RefArray< EffectLayer > m_layers;
};

	}
}

#endif	// traktor_spray_Effect_H
