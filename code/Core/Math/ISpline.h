#ifndef traktor_ISpline_H
#define traktor_ISpline_H

namespace traktor
{

template
< 
	typename Key,
	typename Time,
	typename Value
>
class ISpline
{
public:
	virtual ~ISpline() {}

	virtual Value evaluate(const Time& Tat, const Time& Tend = Time(-1.0f), const Time& stiffness = Time(0.5f)) const = 0;
};

}

#endif	// traktor_ISpline_H
