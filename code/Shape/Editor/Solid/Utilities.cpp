#include "Core/Math/Transform.h"
#include "Shape/Editor/Solid/Utilities.h"

namespace traktor
{
    namespace shape
    {
        namespace
        {

void split(
    const Plane& plane,
    const Winding3& polygon,
    AlignedVector< Winding3 >& outCoplanarFront,
    AlignedVector< Winding3 >& outCoplanarBack,
    AlignedVector< Winding3 >& outFront,
    AlignedVector< Winding3 >& outBack
)
{
    int32_t cf = polygon.classify(plane);
    switch (cf)
    {
    case Winding3::CfFront:
        outFront.push_back(polygon);
        break;

    case Winding3::CfBack:
        outBack.push_back(polygon);
        break;

    case Winding3::CfSpan:
        polygon.split(
            plane,
            outFront.push_back(),
            outBack.push_back()
        );
		T_FATAL_ASSERT(outFront.back().size() >= 3);
		T_FATAL_ASSERT(outBack.back().size() >= 3);
        break;

    case Winding3::CfCoplanar:
        {
            Plane pp;
            if (polygon.getPlane(pp))
            {
                if (dot3(pp.normal(), plane.normal()) >= 0.0f)
                    outCoplanarFront.push_back(polygon);
                else
                    outCoplanarBack.push_back(polygon);
            }
        }
        break;
    }
}

class BspNode
{
public:
	BspNode();

	virtual ~BspNode();

	void invert();

	AlignedVector< Winding3 > clip(const AlignedVector< Winding3 >& polygons) const;

	void clip(const BspNode& other);

	void build(const AlignedVector< Winding3 >& polygons);

	AlignedVector< Winding3 > allPolygons() const;

private:
	Plane m_plane;
	AlignedVector< Winding3 > m_polygons;
	BspNode* m_front;
	BspNode* m_back;
};

BspNode::BspNode()
:   m_front(nullptr)
,   m_back(nullptr)
{
}

BspNode::~BspNode()
{
    delete m_back;
    delete m_front;
}

void BspNode::invert()
{
	m_plane = Plane(
        -m_plane.normal(),
        -m_plane.distance()
    );

	for (auto& polygon : m_polygons)
		polygon.flip();

	if (m_front)
		m_front->invert();
	if (m_back)
		m_back->invert();

	std::swap(m_front, m_back);
}

AlignedVector< Winding3 > BspNode::clip(const AlignedVector< Winding3 >& polygons) const
{
	AlignedVector< Winding3 > front;
	AlignedVector< Winding3 > back;

	for (const auto& polygon : polygons)
	{
		split(
			m_plane,
            polygon,
			front,	// coplanar front
			back,	// coplanar back
			front,	// front
			back	// back
		);
	}

	if (m_front)
		front = m_front->clip(front);
	if (m_back)
		back = m_back->clip(back);
	else
		back.resize(0);

	front.insert(front.end(), back.begin(), back.end());
	return front;
}

void BspNode::clip(const BspNode& other)
{
	m_polygons = other.clip(m_polygons);
	if (m_front)
		m_front->clip(other);
	if (m_back)
		m_back->clip(other);	
}

void BspNode::build(const AlignedVector< Winding3 >& polygons)
{
	if (polygons.empty())
		return;

	size_t i = 0;

	if (!m_front && !m_back)
	{
		while (i < polygons.size() && !polygons[i].getPlane(m_plane))
			++i;
	}
	if (i >= polygons.size())
		return;

	m_polygons.push_back(polygons[i]);

	AlignedVector< Winding3 > front;
	AlignedVector< Winding3 > back;

	for (++i; i < polygons.size(); ++i)
	{
		split(
			m_plane,
            polygons[i],
			m_polygons,	// coplanar front
			m_polygons,	// coplanar back
			front,	// front
			back	// back			
		);
	}

	if (!front.empty())
	{
		if (!m_front)
			m_front = new BspNode();
		m_front->build(front);
	}
	if (!back.empty())
	{
		if (!m_back)
			m_back = new BspNode();
		m_back->build(back);
	}
}

AlignedVector< Winding3 > BspNode::allPolygons() const
{
	AlignedVector< Winding3 > polygons = m_polygons;
	if (m_front)
	{
		AlignedVector< Winding3 > front = m_front->allPolygons();
		polygons.insert(polygons.end(), front.begin(), front.end());
	}
	if (m_back)
	{
		AlignedVector< Winding3 > back = m_back->allPolygons();
		polygons.insert(polygons.end(), back.begin(), back.end());
	}
	return polygons;
}

        }

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

// A | B
AlignedVector< Winding3 > unioon(const AlignedVector< Winding3 >& windingsA, const AlignedVector< Winding3 >& windingsB)
{
 	BspNode A, B;

	A.build(windingsA);
	B.build(windingsB);
	A.clip(B);
	B.clip(A);
	B.invert();
	B.clip(A);
	B.invert();
	A.build(B.allPolygons());

	return A.allPolygons();
}

// A & B == ~(~A | ~B)
AlignedVector< Winding3 > intersection(const AlignedVector< Winding3 >& windingsA, const AlignedVector< Winding3 >& windingsB)
{
	BspNode A, B;

	A.build(windingsA);
	B.build(windingsB);

	A.invert();
	B.clip(A);
	B.invert();
	A.clip(B);
	B.clip(A);
	A.build(B.allPolygons());
	A.invert();

	return A.allPolygons();
}

// A - B == ~(~A | B)
AlignedVector< Winding3 > difference(const AlignedVector< Winding3 >& windingsA, const AlignedVector< Winding3 >& windingsB)
{
	BspNode A, B;

	A.build(windingsA);
	B.build(windingsB);
	A.invert();
	A.clip(B);
	B.clip(A);
	B.invert();
	B.clip(A);
	B.invert();
	A.build(B.allPolygons());
	A.invert();

	return A.allPolygons();
}

    }
}