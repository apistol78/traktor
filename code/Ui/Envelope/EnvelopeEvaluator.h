/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_EnvelopeEvaluator_H
#define traktor_ui_EnvelopeEvaluator_H

#include <vector>
#include "Core/Object.h"
#include "Core/RefArray.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{

class EnvelopeKey;

/*! \brief Envelope evaluator.
 * \ingroup UI
 */
class T_DLLCLASS EnvelopeEvaluator : public Object
{
	T_RTTI_CLASS;

public:
	virtual float evaluate(const RefArray< EnvelopeKey >& keys, float T) = 0;
};

	}
}

#endif	// traktor_ui_EnvelopeEvaluator_H
