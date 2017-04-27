/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_Const_H
#define traktor_Const_H

namespace traktor
{

/*! \ingroup Core */
//@{

const float PI = float(3.1415926535897932384626433832795);
const float HALF_PI = PI / 2.0f;
const float TWO_PI = PI * 2.0f;
const float FUZZY_EPSILON = 1e-6f;

inline float deg2rad(float deg) { return deg * PI / 180.0f; }
inline float rad2deg(float rad) { return rad * 180.0f / PI; }

//@}

}

#endif	// traktor_Const_H
