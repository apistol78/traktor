/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Math/Envelope.h"
#include "Ui/Envelope/EnvelopeEvaluator.h"
#include "Ui/Envelope/EnvelopeKey.h"

namespace traktor
{
	namespace ui
	{

/*! Default envelope evaluator.
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

