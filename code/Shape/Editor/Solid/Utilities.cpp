#include "Core/Math/BspTree.h"
#include "Core/Math/Transform.h"
#include "Shape/Editor/Solid/Utilities.h"

namespace traktor
{
    namespace shape
    {

AlignedVector< Winding3 > transform(const AlignedVector< Winding3 >& windings, const Transform& transform)
{
    AlignedVector< Winding3 > result(windings.size());
    for (uint32_t i = 0; i < windings.size(); ++i)
    {
        const auto& sw = windings[i];
        auto& dw = result[i];

        dw.resize(sw.size());
        for (uint32_t j = 0; j < sw.size(); ++j)
            dw[j] = transform * sw[j].xyz1();
    }
    return result;
}

AlignedVector< Winding3 > invert(const AlignedVector< Winding3 >& windings)
{
    AlignedVector< Winding3 > result(windings.size());
    for (uint32_t i = 0; i < windings.size(); ++i)
    {
        result[i] = windings[i];
        result[i].flip();
    }
    return result;
}

// A | B
AlignedVector< Winding3 > unioon(const AlignedVector< Winding3 >& windingsA, const AlignedVector< Winding3 >& windingsB)
{
    AlignedVector< Winding3 > result;

	// Clip all A to tree B.
    {
        BspTree treeB(windingsB);
        for (const auto& wa : windingsA)
        {
            treeB.clip(wa, [&](const Winding3& w, uint32_t cl, bool splitted)
            {
                if (w.size() >= 3 && cl == Winding3::CfFront)
                    result.push_back(w);
            });
        }
    }

	// Clip all B to tree A.
    {
        BspTree treeA(windingsA);
        for (const auto& wb : windingsB)
        {
            treeA.clip(wb, [&](const Winding3& w, uint32_t cl, bool splitted)
            {
                if (w.size() >= 3 && cl == Winding3::CfFront)
                    result.push_back(w);
            });
        }
    }

    return result;
}

// A & B == ~(~A | ~B)
AlignedVector< Winding3 > intersection(const AlignedVector< Winding3 >& windingsA, const AlignedVector< Winding3 >& windingsB)
{
    auto invWindingsA = invert(windingsA);
    auto invWindingsB = invert(windingsB);
    auto invResult = unioon(invWindingsA, invWindingsB);
    return invert(invResult);
}

// A - B == ~(~A | B)
AlignedVector< Winding3 > difference(const AlignedVector< Winding3 >& windingsA, const AlignedVector< Winding3 >& windingsB)
{
    auto invWindingsA = invert(windingsA);
    auto invResult = unioon(invWindingsA, windingsB);
    return invert(invResult);
}

    }
}