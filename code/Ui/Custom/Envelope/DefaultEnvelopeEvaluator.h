#ifndef traktor_ui_custom_DefaultEnvelopeEvaluator_H
#define traktor_ui_custom_DefaultEnvelopeEvaluator_H

#include "Ui/Custom/Envelope/EnvelopeEvaluator.h"
#include "Ui/Custom/Envelope/EnvelopeKey.h"
#include "Core/Math/Envelope.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

/*! \brief Default envelope evaluator.
 * \ingroup UIC
 */
template < typename Evaluator >
class DefaultEnvelopeEvaluator : public EnvelopeEvaluator
{
public:
	virtual float evaluate(const RefArray< EnvelopeKey >& keys, float T)
	{
		Envelope< float, Evaluator > env;
		for (RefArray< EnvelopeKey >::const_iterator i = keys.begin(); i != keys.end(); ++i)
			env.addKey((*i)->getT(), (*i)->getValue());
		return env[T];
	}
};

		}
	}
}

#endif	// traktor_ui_custom_DefaultEnvelopeEvaluator_H
