#include "Shape/Editor/Solid/PrimitiveEntity.h"
#include "Shape/Editor/Solid/SolidEntity.h"

namespace traktor
{
    namespace shape
    {
        namespace
        {

struct GatherFront
{
	AlignedVector< Winding3 > windings;
	int32_t mask;

	void operator () (uint32_t index, const Winding3& w, int32_t cl, bool splitted)
	{
		if (w.size() < 3)
			return;

		mask |= (1 << cl);
		if (cl == Winding3::CfFront)
			windings.push_back(w);
	}
};

bool calculateUnion(const AlignedVector< Winding3 >& windingsA, const AlignedVector< Winding3 >& windingsB, AlignedVector< Winding3 >& outResult)
{
	BspTree treeA;
	BspTree treeB;

	if (!treeA.build(windingsA) || !treeB.build(windingsB))
		return false;

	// Clip all A to tree B.
	{
		GatherFront visitor;
		for (uint32_t i = 0; i < windingsA.size(); ++i)
		{
			visitor.mask = 0;
			visitor.windings.resize(0);

			treeB.clip(windingsA[i], visitor);

			if (visitor.mask == ((1 << Winding3::CfFront) | (1 << Winding3::CfBack)))
			{
				for (uint32_t j = 0; j < visitor.windings.size(); ++j)
				{
					const Winding3& w = visitor.windings[j];
                    outResult.push_back(w);
				}
			}
			else if (visitor.mask == (1 << Winding3::CfFront))
			{
				const Winding3& w = windingsA[i];
				outResult.push_back(w);
			}
		}
	}

	// Clip all B to tree A.
	{
		GatherFront visitor;
		for (uint32_t i = 0; i < windingsB.size(); ++i)
		{
			visitor.mask = 0;
			visitor.windings.resize(0);

			treeA.clip(windingsB[i], visitor);

			if (visitor.mask == ((1 << Winding3::CfFront) | (1 << Winding3::CfBack)))
			{
				for (uint32_t j = 0; j < visitor.windings.size(); ++j)
				{
					const Winding3& w = visitor.windings[j];
                    outResult.push_back(w);
				}
			}
			else if (visitor.mask == (1 << Winding3::CfFront))
			{
				const Winding3& w = windingsB[i];
				outResult.push_back(w);
			}
		}
	}

    return true;
}

void transform(const AlignedVector< Winding3 >& windings, const Transform& transform, AlignedVector< Winding3 >& outResult)
{
    outResult.resize(windings.size());
    for (uint32_t i = 0; i < windings.size(); ++i)
    {
        const auto& sw = windings[i];
        auto& dw = outResult[i];

        dw.resize(sw.size());
        for (uint32_t j = 0; j < sw.size(); ++j)
        {
            dw[j] = transform * sw[j].xyz1();
        }
    }
}

        }

T_IMPLEMENT_RTTI_CLASS(L"traktor.shape.SolidEntity", SolidEntity, world::GroupEntity)

SolidEntity::SolidEntity(const Transform& transform)
:   world::GroupEntity(transform)
{
}

void SolidEntity::update(const world::UpdateParams& update)
{
    world::GroupEntity::update(update);

    RefArray< PrimitiveEntity > primitiveEntities;
    getEntitiesOf< PrimitiveEntity >(primitiveEntities);

    // Check if any child entity is dirty and if so update our preview geometry.
    bool dirty = false;
    for (auto primitiveEntity : primitiveEntities)
    {
        dirty |= primitiveEntity->isDirty();
        primitiveEntity->resetDirty();
    }
    if (dirty)
    {
        m_windings.resize(0);

        auto it = primitiveEntities.begin();
        if (it != primitiveEntities.end())
        {
            transform((*it)->getWindings(), (*it)->getTransform(), m_windings);
            for (++it; it != primitiveEntities.end(); ++it)
            {
                AlignedVector< Winding3 > windings;
                transform((*it)->getWindings(), (*it)->getTransform(), windings);

                AlignedVector< Winding3 > result;
                calculateUnion(m_windings, windings, result);

                m_windings.swap(result);
            }
        }
    }
}

    }
}
