#ifndef traktor_spark_TesselatorVisitor_H
#define traktor_spark_TesselatorVisitor_H

#include <stack>
#include "Core/Ref.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Matrix33.h"
#include "Core/Math/Vector2.h"
#include "Spark/Editor/Shape/ShapeVisitor.h"

namespace traktor
{
	namespace spark
	{

class Path;
class Style;
struct SubPath;

class TesselatorVisitor : public ShapeVisitor
{
	T_RTTI_CLASS;
	
public:
	struct Listener
	{
		virtual void drawShape(
			const Style* style,
			const AlignedVector< Vector2 >& shape
		) = 0;
	};
	
	TesselatorVisitor(Listener& listener, const Style* style = 0);
	
	virtual void enter(Shape* shape);

	virtual void leave(Shape* shape);

	void freezePath(const Path& path);
	
private:
	Listener& m_listener;
	Ref< const Style > m_style;
	std::stack< Matrix33 > m_transform;
	
	void freezeLinearSubPath(const SubPath& subPath, AlignedVector< Vector2 >& outShape);

	void freezeCubicSubPath(const SubPath& subPath, AlignedVector< Vector2 >& outShape);
};
	
	}
}

#endif	// traktor_spark_TesselatorVisitor_H
