#pragma once

#include "Core/RefArray.h"

namespace traktor
{
	namespace model
	{

class Model;

	}

	namespace shape
	{

void splitModel(const model::Model* model, RefArray< model::Model >& outModels);

	}
}