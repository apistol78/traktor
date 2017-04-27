/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ISpline_H
#define traktor_ISpline_H

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

#endif	// traktor_ISpline_H
