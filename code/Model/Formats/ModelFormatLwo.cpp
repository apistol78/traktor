//#include <algorithm>
//#include <limits>
//#include <cstdio>
//#include "Core/Io/Path.h"
//#include "Core/Log/Log.h"
//#include "Core/Math/Const.h"
#include "Core/Misc/String.h"
//#include "Core/Misc/TString.h"
//#include "Core/Serialization/ISerializable.h"
//#include "Core/Settings/PropertyBoolean.h"
//#include "Core/Thread/Acquire.h"
//#include "Core/Thread/Semaphore.h"
#include "Model/Model.h"
#include "Model/Formats/ModelFormatLwo.h"
#include "Model/Formats/LwLayer.h"
#include "Model/Formats/LwObject.h"
#include "Model/Formats/LwSurface.h"
#include "Model/Formats/LwTags.h"

namespace traktor
{
	namespace model
	{
		namespace
		{
//
//#define ID_WGHT	LWID_('W','G','H','T')
//#define ID_SPOT	LWID_('S','P','O','T')	// Absolute
//#define ID_MORF	LWID_('M','O','R','F')	// Relative
//#define ID_RGBA LWID_('R','G','B','A')	// Vertex color map.
//#define ID_RGB  LWID_('R','G','B',' ')	// Vertex color map (no alpha).
//
//Semaphore s_lock;	//< LWO parser isn't thread-safe; it's having global variables for tracking IO bytes.
//
//const lwClip* findLwClip(const lwObject* lwo, int clipIndex)
//{
//	const lwClip* clip = lwo->clip;
//	while (clip && clip->index != clipIndex)
//		clip = clip->next;
//	return clip;
//}
//
//const lwTexture* getLwTexture(const lwTexture* tex)
//{
//	for (; tex; tex = tex->next)
//	{
//		if (tex->type == ID_IMAP && tex->param.imap.vmap_name)
//			break;
//	}
//	return tex;
//}
//
//std::wstring fixTextureFileName(const std::wstring& fileName)
//{
//	std::wstring textureName = fileName;
//	size_t pos;
//
//	pos = textureName.find_last_of(L'/');
//	if (pos != std::wstring::npos)
//		textureName = textureName.substr(pos + 1);
//
//	pos = textureName.find(L'.');
//	if (pos != std::wstring::npos)
//		textureName = textureName.substr(0, pos);
//
//	return textureName;
//}
//
//const lwVMapPt* findLwVMapPt(const lwVMapPt* vmaps, int nvmaps, const std::string& vmapName)
//{
//	for (int i = 0; i < nvmaps; ++i)
//	{
//		if (std::string(vmaps[i].vmap->name) == vmapName)
//			return &vmaps[i];
//	}
//	return 0;
//}

bool createMaterials(const LwObject* lwo, Model* outModel)
{
	const RefArray< LwSurface >& surfaces = lwo->getSurfaces();
	for (RefArray< LwSurface >::const_iterator i = surfaces.begin(); i != surfaces.end(); ++i)
	{
		const LwSurface* surface = *i;
		T_ASSERT (surface);

		Material material;
		material.setName(surface->getName());

		//const lwTexture* texDiffuse = getLwTexture(surface->color.tex);
		//if (texDiffuse)
		//{
		//	const lwClip* clip = findLwClip(lwo, texDiffuse->param.imap.cindex);
		//	if (clip)
		//	{
		//		Material::BlendOperator diffuseBlendOperator = Material::BoDecal;
		//		switch (surface->color.tex->opac_type)
		//		{
		//		case 0:
		//			diffuseBlendOperator = Material::BoDecal;
		//			break;
		//		case 3:
		//			diffuseBlendOperator = Material::BoMultiply;
		//			break;
		//		case 5:
		//			diffuseBlendOperator = Material::BoAlpha;
		//			break;
		//		case 7:
		//			diffuseBlendOperator = Material::BoAdd;
		//			break;
		//		default:
		//			log::warning << L"Unknown opacity type (" << surface->color.tex->opac_type << L") on surface \"" << material.getName() << L"\"" << Endl;
		//		}

		//		std::wstring textureName = fixTextureFileName(mbstows(clip->source.still.name));
		//		material.setDiffuseMap(textureName);
		//		material.setBlendOperator(diffuseBlendOperator);
		//	}
		//	else
		//		T_DEBUG(L"No diffuse texture clip for surface \"" << material.getName() << L"\"");
		//}

		//const lwTexture* texSpecular = getLwTexture(surface->specularity.tex);
		//if (texSpecular)
		//{
		//	const lwClip* clip = findLwClip(lwo, texSpecular->param.imap.cindex);
		//	if (clip)
		//	{
		//		std::wstring textureName = fixTextureFileName(mbstows(clip->source.still.name));
		//		material.setSpecularMap(textureName);
		//	}
		//	else
		//		T_DEBUG(L"No specular texture clip for surface \"" << mbstows(surface->name) << L"\"");
		//}

		//const lwTexture* texEmissive = getLwTexture(surface->luminosity.tex);
		//if (texEmissive)
		//{
		//	const lwClip* clip = findLwClip(lwo, texEmissive->param.imap.cindex);
		//	if (clip)
		//	{
		//		std::wstring textureName = fixTextureFileName(mbstows(clip->source.still.name));
		//		material.setEmissiveMap(textureName);
		//	}
		//	else
		//		T_DEBUG(L"No emissive texture clip for surface \"" << mbstows(surface->name) << L"\"");
		//}

		//const lwTexture* texReflective = getLwTexture(surface->reflection.val.tex);
		//if (texReflective)
		//{
		//	const lwClip* clip = findLwClip(lwo, texReflective->param.imap.cindex);
		//	if (clip)
		//	{
		//		std::wstring textureName = fixTextureFileName(mbstows(clip->source.still.name));
		//		material.setReflectiveMap(textureName);
		//	}
		//	else
		//		T_DEBUG(L"No reflective texture clip for surface \"" << mbstows(surface->name) << L"\"");
		//}

		//const lwTexture* texBump = getLwTexture(surface->bump.tex);
		//if (texBump)
		//{
		//	const lwClip* clip = findLwClip(lwo, texBump->param.imap.cindex);
		//	if (clip)
		//	{
		//		std::wstring textureName = fixTextureFileName(mbstows(clip->source.still.name));
		//		material.setNormalMap(textureName);
		//	}
		//	else
		//		T_DEBUG(L"No bump texture clip for surface \"" << mbstows(surface->name) << L"\"");
		//}

		material.setColor(Color4ub(
			uint8_t(surface->getColor().val.getRed() * 255.0f),
			uint8_t(surface->getColor().val.getGreen() * 255.0f),
			uint8_t(surface->getColor().val.getBlue() * 255.0f),
			255 //uint8_t((1.0f - surface->transparency.val.val) * 255.0f)
		));

		//if (surface->transparency.val.val >= FUZZY_EPSILON)
		//	material.setBlendOperator(Material::BoAlpha);

		//material.setDiffuseTerm(surface->diffuse.val);
		//material.setSpecularTerm(surface->specularity.val);
		//material.setSpecularRoughness(surface->glossiness.val);
		//material.setEmissive(surface->luminosity.val);
		//material.setReflective(surface->reflection.val.val);
		//material.setRimLightIntensity(surface->glow.val);

		//if ((surface->sideflags & 3) == 3)
		//	material.setDoubleSided(true);

		//if (surface->comment)
		//	material.setProperty< PropertyBoolean >(mbstows(surface->comment), true);

		outModel->addMaterial(material);
	}

	return true;
}

bool createMesh(const LwObject* lwo, Model* outModel, uint32_t importFlags)
{
	const RefArray< LwLayer >& layers = lwo->getLayers();

	// Count number of primitives.
	uint32_t pointCount = 0;
	uint32_t polygonCount = 0;

	for (RefArray< LwLayer >::const_iterator i = layers.begin(); i != layers.end(); ++i)
	{
		pointCount += (*i)->getPoints().size();
		polygonCount += (*i)->getPolygons().size();
	}

	// Convert positions.
	if (importFlags & ModelFormat::IfMeshPositions)
	{
		outModel->reservePositions(pointCount);

		for (RefArray< LwLayer >::const_iterator i = layers.begin(); i != layers.end(); ++i)
		{
			const AlignedVector< Vector4 >& points = (*i)->getPoints();
			for (AlignedVector< Vector4 >::const_iterator j = points.begin(); j != points.end(); ++j)
				outModel->addPosition(j->xyz1());
		}
	}

	/*
	// Convert blend targets.
	if (importFlags & ModelFormat::IfMeshBlendTargets)
	{
		positionBase = 0;
		for (lwLayer* layer = lwo->layer; layer; layer = layer->next)
		{
			for (int i = 0; i < layer->point.count; ++i)
			{
				lwPoint* pnt = layer->point.pt + i;
				for (int j = 0; j < pnt->nvmaps; ++j)
				{
					if (pnt->vm[j].vmap->type == ID_SPOT || pnt->vm[j].vmap->type == ID_MORF)
					{
						uint32_t blendTargetIndex = outModel->addBlendTarget(mbstows(pnt->vm[j].vmap->name));

						Vector4 basePosition = outModel->getPosition(positionBase + i);
						Vector4 targetPosition(
							pnt->vm[j].vmap->val[pnt->vm[j].index][0],
							pnt->vm[j].vmap->val[pnt->vm[j].index][1],
							pnt->vm[j].vmap->val[pnt->vm[j].index][2],
							1.0f
						);
						if (pnt->vm[j].vmap->type == ID_MORF)
							targetPosition += basePosition.xyz0();

						outModel->setBlendTargetPosition(
							blendTargetIndex,
							positionBase + i,
							targetPosition
						);
					}
				}
			}
			positionBase += layer->point.count;
		}
	}
	*/

	// Convert polygons.
	if (importFlags & (ModelFormat::IfMeshPolygons | ModelFormat::IfMeshVertices))
	{
		if (importFlags & ModelFormat::IfMeshPolygons)
			outModel->reservePolygons(polygonCount);

		uint32_t pointBase = 0;

		for (RefArray< LwLayer >::const_iterator i = layers.begin(); i != layers.end(); ++i)
		{
			const AlignedVector< LwPolygon >& polygons = (*i)->getPolygons();
			for (AlignedVector< LwPolygon >::const_iterator j = polygons.begin(); j != polygons.end(); ++j)
			{
				const LwPolygon& pol = *j;

				// Ignore all polygons which aren't plain faces.
				if (pol.type != LwTags::FACE)
					continue;

				/*
				const lwSurface* surf = pol->surf;
				const lwTexture* texDiffuse = getLwTexture(surf->color.tex);

				int materialIndex = 0;
				for (lwSurface* s = lwo->surf; s && s != surf; s = s->next)
					materialIndex++;
				*/

				Polygon polygon;
				polygon.setMaterial(pol.surfaceIndex);
				polygon.setSmoothGroup(pol.smoothGroup);
				/*
				polygon.setNormal(outModel->addUniqueNormal(Vector4(
					pol->norm[0],
					pol->norm[1],
					pol->norm[2]
				)));
				*/

				if (importFlags & ModelFormat::IfMeshVertices)
				{
					for (int32_t j = int32_t(pol.indices.size()) - 1; j >= 0; --j)
					{
						Vertex vertex;
						vertex.setPosition(pointBase + pol.indices[j]);
						/*
						vertex.setNormal(outModel->addUniqueNormal(Vector4(
							pol->v[j].norm[0],
							pol->v[j].norm[1],
							pol->v[j].norm[2]
						)));
						*/

						/*
						// Vertex colors.
						for (int32_t k = 0; k < pnt->nvmaps; ++k)
						{
							const lwVMapPt* vc = &pnt->vm[k];
							if (vc->vmap->type == ID_RGBA)
							{
								const float* color = vc->vmap->val[vc->index];
								vertex.setColor(outModel->addUniqueColor(Vector4(
									color[0],
									color[1],
									color[2],
									color[3]
								)));
								break;
							}
							else if (vc->vmap->type == ID_RGB)
							{
								const float* color = vc->vmap->val[vc->index];
								vertex.setColor(outModel->addUniqueColor(Vector4(
									color[0],
									color[1],
									color[2],
									1.0f
								)));
								break;
							}
						}

						// UV maps.
						uint32_t channel = 0;
						for (const lwTexture* tex = texDiffuse; tex; tex = tex->next)
						{
							const lwVMapPt* vpt = findLwVMapPt(pol->v[j].vm, pol->v[j].nvmaps, tex->param.imap.vmap_name);
							if (!vpt)
								vpt = findLwVMapPt(pnt->vm, pnt->nvmaps, tex->param.imap.vmap_name);

							if (vpt)
							{
								float u = vpt->vmap->val[vpt->index][0];
								float v = vpt->vmap->val[vpt->index][1];

								vertex.setTexCoord(channel, outModel->addUniqueTexCoord(Vector2(
									u,
									1.0f - v
								)));
							}
							else
								log::warning << L"Vertex " << j << L" doesn't exist in UV map \"" << mbstows(tex->param.imap.vmap_name) << L"\"" << Endl;

							++channel;
						}

						// Convert weight maps into bones and influences.
						if (importFlags & ModelFormat::IfMeshBlendWeights)
						{
							// Collect weight map references for this point.
							std::vector< lwVMapPt > weightRefs;
							for (int i = 0; i < pnt->nvmaps; ++i)
							{
								if (pnt->vm[i].vmap->type == ID_WGHT)
									weightRefs.push_back(pnt->vm[i]);
							}

							// Add weights to vertex, also allocate bone index.
							for (std::vector< lwVMapPt >::iterator i = weightRefs.begin(); i != weightRefs.end(); ++i)
							{
								int jointIndex = outModel->addJoint(mbstows(i->vmap->name));
								float jointInfluence = i->vmap->val[i->index][0];
								vertex.setJointInfluence(jointIndex, jointInfluence);
							}
						}
						*/

						int32_t vertexId = outModel->addUniqueVertex(vertex);
						if (importFlags & ModelFormat::IfMeshPolygons)
							polygon.addVertex(vertexId);
					}
				}

				if (importFlags & ModelFormat::IfMeshPolygons)
					outModel->addPolygon(polygon);
			}

			pointBase += (*i)->getPoints().size();
		}
	}

	return true;
}

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.model.ModelFormatLwo", 0, ModelFormatLwo, ModelFormat)

void ModelFormatLwo::getExtensions(std::wstring& outDescription, std::vector< std::wstring >& outExtensions) const
{
	outDescription = L"Lightwave Object";
	outExtensions.push_back(L"lwo");
	outExtensions.push_back(L"lw");
}

bool ModelFormatLwo::supportFormat(const std::wstring& extension) const
{
	return 
		compareIgnoreCase< std::wstring >(extension, L"lwo") == 0 ||
		compareIgnoreCase< std::wstring >(extension, L"lw") == 0;
}

Ref< Model > ModelFormatLwo::read(IStream* stream, uint32_t importFlags) const
{
	Ref< LwObject > lwo = new LwObject();

	if (!lwo->read(stream))
		return 0;

	Ref< Model > md = new Model();

	if (importFlags & IfMaterials)
	{
		if (!createMaterials(lwo, md))
			return 0;
	}

	if (importFlags & IfMesh)
	{
		if (!createMesh(lwo, md, importFlags))
			return 0;
	}

	return md;
}

bool ModelFormatLwo::write(IStream* stream, const Model* model) const
{
	return false;
}

	}
}
