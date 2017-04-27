/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_spark_ColorTransform_H
#define traktor_spark_ColorTransform_H

#include "Core/Config.h"

namespace traktor
{
	namespace spark
	{

/*! \brief Color transformation.
 * \ingroup Spark
 *
 * R = r * red[0] + red[1]
 * G = g * green[0] + green[1]
 * B = b * blue[0] + blue[1]
 * A = a * alpha[0] + alpha[1]
 *
 */
class ColorTransform
{
public:
	float red[2];
	float green[2];
	float blue[2];
	float alpha[2];

	ColorTransform()
	{
		red  [0] = 1.0f; red  [1] = 0.0f;
		green[0] = 1.0f; green[1] = 0.0f;
		blue [0] = 1.0f; blue [1] = 0.0f;
		alpha[0] = 1.0f; alpha[1] = 0.0f;
	}
};

	}
}

#endif	// traktor_spark_ColorTransform_H
