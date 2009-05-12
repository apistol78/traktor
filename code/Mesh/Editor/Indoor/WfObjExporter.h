#ifndef traktor_mesh_WfObjExporter_H
#define traktor_mesh_WfObjExporter_H

#include <map>
#include <vector>
#include <string>
#include "Core/Math/Winding.h"

namespace traktor
{
	namespace mesh
	{

class WfObjExporter
{
public:
	void addPolygon(const std::wstring& material, const Winding& polygon);

	void addPolygons(const std::wstring& material, const std::vector< Winding >& polygons);

	void write(const std::wstring& fileName);

private:
	std::map< std::wstring, std::vector< Winding > > m_polygons;
};

	}
}

#endif	// traktor_mesh_WfObjExporter_H
