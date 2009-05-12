#include "Mesh/Editor/Indoor/WfObjExporter.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/Stream.h"
#include "Core/Io/FileOutputStream.h"
#include "Core/Io/AnsiEncoding.h"

namespace traktor
{
	namespace mesh
	{

void WfObjExporter::addPolygon(const std::wstring& material, const Winding& polygon)
{
	m_polygons[material].push_back(polygon);
}

void WfObjExporter::addPolygons(const std::wstring& material, const std::vector< Winding >& polygons)
{
	m_polygons[material].insert(m_polygons[material].begin(), polygons.begin(), polygons.end());
}

void WfObjExporter::write(const std::wstring& fileName)
{
	Ref< Stream > file = FileSystem::getInstance().open(fileName, File::FmWrite);
	T_ASSERT (file);

	FileOutputStream s(file, gc_new< AnsiEncoding >());

	s << L"o Dummy.obj" << Endl;
	s << Endl;
	s << L"#	             Vertex list" << Endl;
	s << Endl;

	for (std::map< std::wstring, std::vector< Winding > >::iterator i = m_polygons.begin(); i != m_polygons.end(); ++i)
	{
		const std::vector< Winding >& polygons = i->second;
		for (size_t i = 0; i < polygons.size(); ++i)
		{
			for (size_t j = 0; j < polygons[i].points.size(); ++j)
				s << L"v " << polygons[i].points[j].x() << L" " << polygons[i].points[j].y() << L" " << -polygons[i].points[j].z() << Endl;
		}
	}

	s << Endl;
	s << L"#	    Point/Line/Face list" << Endl;
	s << Endl;

	uint32_t idx = 0;

	for (std::map< std::wstring, std::vector< Winding > >::iterator i = m_polygons.begin(); i != m_polygons.end(); ++i)
	{
		s << L"usemtl " << i->first << Endl;
		const std::vector< Winding >& polygons = i->second;
		for (size_t i = 0; i < polygons.size(); ++i)
		{
			s << L"f";
			for (size_t j = 0; j < polygons[i].points.size(); ++j)
				s << L" " << ++idx; 
			s << Endl;
		}
	}

	s << Endl;
	s << L"#	             End of file" << Endl;

	s.close();
}

	}
}
