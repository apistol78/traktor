#pragma once

namespace traktor
{

template
<
	typename Value
>
class ISpline
{
public:
	virtual ~ISpline() {}

	virtual Value evaluate(float T) const = 0;
};

}

