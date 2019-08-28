#pragma once

#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Winding3.h"

namespace traktor
{

class Transform;

    namespace shape
    {

/*!
 * \ingroup Shape
 */
AlignedVector< Winding3 > transform(const AlignedVector< Winding3 >& windings, const Transform& transform);

/*!
 * \ingroup Shape
 */
// AlignedVector< Winding3 > invert(const AlignedVector< Winding3 >& windings);

/*!
 * \ingroup Shape
 */
AlignedVector< Winding3 > unioon(const AlignedVector< Winding3 >& windingsA, const AlignedVector< Winding3 >& windingsB);

/*!
 * \ingroup Shape
 */
AlignedVector< Winding3 > intersection(const AlignedVector< Winding3 >& windingsA, const AlignedVector< Winding3 >& windingsB);

/*!
 * \ingroup Shape
 */
AlignedVector< Winding3 > difference(const AlignedVector< Winding3 >& windingsA, const AlignedVector< Winding3 >& windingsB);

    }
}