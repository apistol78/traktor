#ifndef traktor_ui_custom_DefaultEnvelopeEvaluator_H
#define traktor_ui_custom_DefaultEnvelopeEvaluator_H

#include "Core/Math/Envelope.h"
#include "Ui/Custom/Envelope/EnvelopeEvaluator.h"
#include "Ui/Custom/Envelope/EnvelopeKey.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

/*! \brief Default envelope evaluator.
 * \ingroup UIC
 */
template
<
	template < typename ValueType > class Evaluator
>
class DefaultEnvelopeEvaluator : public EnvelopeEvaluator
{
public:
	virtual float evaluate(const RefArray< EnvelopeKey >& keys, float T)
	{
		Envelope< float, Evaluator< float > > envelope;
		for (RefArray< EnvelopeKey >::const_iterator i = keys.begin(); i != keys.end(); ++i)
			envelope.addKey((*i)->getT(), (*i)->getValue());
		return envelope(T);
	}
};

		}
	}
}

#endif	// traktor_ui_custom_DefaultEnvelopeEvaluator_H
