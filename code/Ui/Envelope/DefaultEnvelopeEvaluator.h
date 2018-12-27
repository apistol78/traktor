/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_DefaultEnvelopeEvaluator_H
#define traktor_ui_DefaultEnvelopeEvaluator_H

#include "Core/Math/Envelope.h"
#include "Ui/Envelope/EnvelopeEvaluator.h"
#include "Ui/Envelope/EnvelopeKey.h"

namespace traktor
{
	namespace ui
	{

/*! \brief Default envelope evaluator.
 * \ingroup UI
 */
template
<
	template < typename ValueType > class Evaluator
>
class DefaultEnvelopeEvaluator : public EnvelopeEvaluator
{
public:
	virtual float evaluate(const RefArray< EnvelopeKey >& keys, float T) override final
	{
		Envelope< float, Evaluator< float > > envelope;
		for (RefArray< EnvelopeKey >::const_iterator i = keys.begin(); i != keys.end(); ++i)
			envelope.addKey((*i)->getT(), (*i)->getValue());
		return envelope(T);
	}
};

	}
}

#endif	// traktor_ui_DefaultEnvelopeEvaluator_H
