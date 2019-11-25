#include "Model/Model.h"
#include "Model/Operations/Unweld.h"

namespace traktor
{
	namespace model
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.model.Unweld", Unweld, IModelOperation)

bool Unweld::apply(Model& model) const
{
	AlignedVector< Vertex > inputVertices = model.getVertices();

	model.setVertices(AlignedVector< Vertex >());
	for (auto& polygon : model.getPolygons())
	{
		auto polverts = polygon.getVertices();
		for (auto& polvert : polverts)
			polvert = model.addVertex(inputVertices[polvert]);
		polygon.setVertices(polverts);
	}

	return true;
}

void Unweld::serialize(ISerializer& s)
{
}

	}
}
