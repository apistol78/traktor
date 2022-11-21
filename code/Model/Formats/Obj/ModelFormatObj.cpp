/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/BufferedStream.h"
#include "Core/Io/FileOutputStream.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/StringReader.h"
#include "Core/Io/AnsiEncoding.h"
#include "Core/Misc/String.h"
#include "Core/Misc/Split.h"
#include "Model/Model.h"
#include "Model/Formats/Obj/ModelFormatObj.h"

namespace traktor::model
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.model.ModelFormatObj", 0, ModelFormatObj, ModelFormat)

void ModelFormatObj::getExtensions(std::wstring& outDescription, std::vector< std::wstring >& outExtensions) const
{
	outDescription = L"Wavefront Object";
	outExtensions.push_back(L"obj");
}

bool ModelFormatObj::supportFormat(const std::wstring& extension) const
{
	return compareIgnoreCase(extension, L"obj") == 0;
}

Ref< Model > ModelFormatObj::read(const Path& filePath, const std::wstring& filter) const
{
	Ref< IStream > stream = FileSystem::getInstance().open(filePath, File::FmRead);
	if (!stream)
		return nullptr;

	BufferedStream bs(stream);
	StringReader sr(&bs, new AnsiEncoding());
	AlignedVector< float > fvalues;
	std::wstring str;

	Ref< Model > md = new Model();
	uint32_t materialId = c_InvalidIndex;

	while (sr.readLine(str) >= 0)
	{
		str = trim(str);
		if (str.empty() || str[0] == L'#')
			continue;

		if (startsWith(str, L"v "))
		{
			fvalues.resize(0);
			if (Split< std::wstring, float >::any(str.substr(2), L" \t", fvalues) >= 3)
			{
				md->addPosition(Vector4(
					-fvalues[0],
					fvalues[1],
					fvalues[2],
					1.0f
				));
			}
		}
		else if (startsWith(str, L"vt "))
		{
			fvalues.resize(0);
			if (Split< std::wstring, float >::any(str.substr(2), L" \t", fvalues) >= 2)
			{
				md->addTexCoord(Vector2(
					fvalues[0],
					1.0f - fvalues[1]
				));
			}
		}
		else if (startsWith(str, L"vn "))
		{
			fvalues.resize(0);
			if (Split< std::wstring, float >::any(str.substr(2), L" \t", fvalues) >= 3)
			{
				md->addNormal(Vector4(
					-fvalues[0],
					fvalues[1],
					fvalues[2],
					0.0f
				));
			}
		}
		else if (startsWith(str, L"f "))
		{
			AlignedVector< std::wstring > values;
			if (Split< std::wstring >::any(str.substr(2), L" \t", values) > 0)
			{
				Polygon polygon;
				polygon.setMaterial(materialId);

				for (AlignedVector< std::wstring >::const_iterator i = values.begin(); i != values.end(); ++i)
				{
					AlignedVector< int32_t > indices;
					if (Split< std::wstring, int32_t >::any(*i, L"/", indices, true) > 0)
					{
						Vertex vertex;
						vertex.setPosition(indices[0] - 1);
						if (indices.size() > 1 && indices[1] > 0)
							vertex.setTexCoord(0, indices[1] - 1);
						if (indices.size() > 2 && indices[2] > 0)
							vertex.setNormal(indices[2] - 1);
						polygon.addVertex(md->addUniqueVertex(vertex));
					}
				}

				md->addUniquePolygon(polygon);
			}
		}
		else if (startsWith(str, L"usemtl ") || startsWith(str, L"g "))
		{
			materialId = c_InvalidIndex;

			std::wstring materialName = str.substr(7);
			const AlignedVector< Material >& materials = md->getMaterials();
			for (uint32_t i = 0; i < uint32_t(materials.size()); ++i)
			{
				if (materials[i].getName() == materialName)
				{
					materialId = i;
					break;
				}
			}

			if (materialId == c_InvalidIndex)
			{
				Material material;
				material.setName(str.substr(7));
				materialId = md->addMaterial(material);
			}
		}
	}

	stream->close();
	return md;
}

bool ModelFormatObj::write(const Path& filePath, const Model* model) const
{
	Ref< IStream > stream = FileSystem::getInstance().open(filePath, File::FmWrite);
	if (!stream)
		return false;

	BufferedStream bs(stream, 512 * 1024);
	FileOutputStream s(&bs, new AnsiEncoding());

	s << L"o unnamed.obj" << Endl;
	s << Endl;
	s << L"#	             Vertex list" << Endl;
	s << Endl;

	for (auto position : model->getPositions())
		s << L"v " << position.x() << L" " << position.y() << L" " << position.z() << Endl;

	for (auto texCoord : model->getTexCoords())
		s << L"vt " << texCoord.x << L" " << (1.0f - texCoord.y) << Endl;

	for (auto normal : model->getNormals())
		s << L"vn " << normal.x() << L" " << normal.y() << L" " << normal.z() << Endl;

	s << Endl;
	s << L"#	    Point/Line/Face list" << Endl;
	s << Endl;

	const auto& materials = model->getMaterials();
	if (!materials.empty())
	{
		for (uint32_t material = 0; material < uint32_t(materials.size()); ++material)
		{
			s << L"usemtl " << materials[material].getName() << Endl;
			for (const auto& polygon : model->getPolygons())
			{
				if (polygon.getMaterial() != material)
					continue;

				s << L"f";
				const auto& vertices = polygon.getVertices();
				for (int32_t i = (int32_t)vertices.size() - 1; i >= 0; --i)
				{
					const Vertex& vertex = model->getVertex(vertices[i]);

					s << L" " << vertex.getPosition() + 1;
					if (vertex.getTexCoord(1) != c_InvalidIndex)
						s << L"/" << vertex.getTexCoord(1) + 1;
					else if (vertex.getTexCoord(0) != c_InvalidIndex)
						s << L"/" << vertex.getTexCoord(0) + 1;
					else
						s << L"/";

					if (vertex.getNormal() != c_InvalidIndex)
						s << L"/" << vertex.getNormal() + 1;
					else
						s << L"/";
				}
				s << Endl;
			}
			s << Endl;
		}
	}
	else
	{
		for (const auto& polygon : model->getPolygons())
		{
			s << L"f";
			const auto& vertices = polygon.getVertices();
			for (int32_t i = (int32_t)vertices.size() - 1; i >= 0; --i)
			{
				const Vertex& vertex = model->getVertex(vertices[i]);

				s << L" " << vertex.getPosition() + 1;
				if (vertex.getTexCoord(0) != c_InvalidIndex)
					s << L"/" << vertex.getTexCoord(0) + 1;
				else
					s << L"/";
				if (vertex.getNormal() != c_InvalidIndex)
					s << L"/" << vertex.getNormal() + 1;
				else
					s << L"/";
			}
			s << Endl;
		}
		s << Endl;
	}

	s << L"#	             End of file" << Endl;

	s.close();
	return true;
}

}
