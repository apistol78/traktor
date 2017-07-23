/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Io/BufferedStream.h"
#include "Core/Io/FileOutputStream.h"
#include "Core/Io/StringReader.h"
#include "Core/Io/AnsiEncoding.h"
#include "Core/Misc/String.h"
#include "Core/Misc/Split.h"
#include "Model/Model.h"
#include "Model/Formats/ModelFormatStl.h"

namespace traktor
{
	namespace model
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.model.ModelFormatStl", 0, ModelFormatStl, ModelFormat)

void ModelFormatStl::getExtensions(std::wstring& outDescription, std::vector< std::wstring >& outExtensions) const
{
	outDescription = L"STL object";
	outExtensions.push_back(L"stl");
}

bool ModelFormatStl::supportFormat(const std::wstring& extension) const
{
	return compareIgnoreCase< std::wstring >(extension, L"stl") == 0;
}

Ref< Model > ModelFormatStl::read(IStream* stream, uint32_t importFlags) const
{
	BufferedStream bs(stream);
	StringReader sr(&bs, new AnsiEncoding());
	std::wstring str;

	sr.readLine(str);
	if (!startsWith< std::wstring >(str, L"solid "))
		return 0;

	Ref< Model > md = new Model();

	Material material;
	material.setName(L"Default");
	uint32_t materialId = md->addMaterial(material);

	for (;;)
	{
		if (sr.readLine(str) <= 0)
			break;

		str = trim(str);
		if (startsWith< std::wstring >(str, L"facet normal "))
		{
			sr.readLine(str); str = trim(str);
			if (!startsWith< std::wstring >(str, L"outer loop"))
				return 0;

			Polygon pol;
			pol.setMaterial(materialId);

			for (;;)
			{
				sr.readLine(str); str = trim(str);
				if (!startsWith< std::wstring >(str, L"vertex "))
					break;

				str = str.substr(7);

				std::vector< float > values;
				if (Split< std::wstring, float >::any(str, L" ", values) < 3)
					return 0;

				uint32_t p = md->addPosition(Vector4(values[0], values[2], values[1]));
				uint32_t v = md->addVertex(Vertex(p));

				pol.addVertex(v);
			}

			md->addPolygon(pol);
		}	
	}

	stream->close();
	return md;
}

bool ModelFormatStl::write(IStream* stream, const Model* model) const
{
	return false;
}

	}
}
