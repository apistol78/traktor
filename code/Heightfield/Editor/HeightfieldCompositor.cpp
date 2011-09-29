#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Io/Writer.h"
#include "Core/Log/Log.h"
#include "Database/Instance.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"
#include "Drawing/Filters/ScaleFilter.h"
#include "Heightfield/Editor/HeightfieldAsset.h"
#include "Heightfield/Editor/HeightfieldCompositor.h"
#include "Heightfield/Editor/HeightfieldLayer.h"

namespace traktor
{
	namespace hf
	{
		namespace
		{

drawing::PixelFormat s_pfRaw16(16, 16, 0, 0, 0, 0, 0, 0, 0, false, false);

Ref< drawing::Image > readRawTerrain(IStream* stream)
{
	uint32_t fileSize = stream->available();

	const uint32_t heightByteSize = 2;

	uint32_t heights = fileSize / heightByteSize;
	uint32_t size = uint32_t(std::sqrt(float(heights)));

	Ref< drawing::Image > image = new drawing::Image(
		s_pfRaw16,
		size,
		size
	);

	stream->read(image->getData(), fileSize);
	stream->close();

	return image;
}

Ref< drawing::Image > readRawTerrain(const Path& fileName)
{
	Ref< IStream > file = FileSystem::getInstance().open(fileName, File::FmRead);
	return file ? readRawTerrain(file) : 0;
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.hf.HeightfieldCompositor", HeightfieldCompositor, Object)

Ref< HeightfieldCompositor > HeightfieldCompositor::createFromInstance(const db::Instance* assetInstance, const std::wstring& assetPath)
{
	Ref< HeightfieldAsset > asset = checked_type_cast< HeightfieldAsset*, true >(assetInstance->getObject< HeightfieldAsset >());
	if (!asset)
		return 0;

	// Load base layer as image.
	Path fileName = FileSystem::getInstance().getAbsolutePath(assetPath, asset->getFileName());
	Ref< drawing::Image > image = readRawTerrain(fileName);
	if (!image)
		return 0;

	// Rescale base layer to fit 2^x.
	uint32_t size = image->getWidth();
	size = ((size / asset->getDetailSkip() + asset->getPatchDim() - 1) / asset->getPatchDim()) * asset->getPatchDim() * asset->getDetailSkip();
	if (size < asset->getPatchDim() || (size / asset->getDetailSkip()) % asset->getPatchDim() != 0)
	{
		log::error << L"Invalid patch dimension or detail skip value in heightfield asset" << Endl;
		return 0;
	}

	drawing::ScaleFilter scaleFilter(
		size,
		size,
		drawing::ScaleFilter::MnAverage,
		drawing::ScaleFilter::MgLinear
	);
	image = image->applyFilter(&scaleFilter);
	T_ASSERT (image);

	// Create compositor instance.
	Ref< HeightfieldCompositor > compositor = new HeightfieldCompositor();

	compositor->m_baseLayer = HeightfieldLayer::createFromImage(image);
	if (!compositor->m_baseLayer)
		return 0;

	// Read offset layer.
	Ref< IStream > offsetStream = assetInstance->readData(L"Offset");
	if (offsetStream)
	{
		Ref< drawing::Image > offsetImage = readRawTerrain(offsetStream);
		if (!offsetImage)
			return 0;

		if (offsetImage->getWidth() != size)
		{
			drawing::ScaleFilter scaleOffsetFilter(
				size,
				size,
				drawing::ScaleFilter::MnAverage,
				drawing::ScaleFilter::MgLinear
			);
			offsetImage = offsetImage->applyFilter(&scaleOffsetFilter);
			T_ASSERT (offsetImage);
		}

		compositor->m_offsetLayer = HeightfieldLayer::createFromImage(offsetImage);
	}

	if (!compositor->m_offsetLayer)
	{
		compositor->m_offsetLayer = HeightfieldLayer::createEmpty(size, 32767);
		T_ASSERT (compositor->m_offsetLayer);
	}

	return compositor;
}

bool HeightfieldCompositor::saveInstanceLayers(db::Instance* assetInstance) const
{
	if (!assetInstance->checkout())
		return false;

	Ref< IStream > offsetStream = assetInstance->writeData(L"Offset");
	if (!offsetStream)
	{
		assetInstance->revert();
		return false;
	}

	const height_t* heights = m_offsetLayer->getHeights();
	uint32_t size = m_offsetLayer->getSize();

	Writer(offsetStream).write(
		heights,
		size * size,
		sizeof(height_t)
	);
	offsetStream->close();

	if (!assetInstance->commit())
	{
		assetInstance->revert();
		return false;
	}

	return true;
}

HeightfieldLayer* HeightfieldCompositor::getBaseLayer()
{
	return m_baseLayer;
}

HeightfieldLayer* HeightfieldCompositor::getOffsetLayer()
{
	return m_offsetLayer;
}

Ref< HeightfieldLayer > HeightfieldCompositor::mergeLayers() const
{
	uint32_t size = m_baseLayer->getSize();

	Ref< HeightfieldLayer > mergedLayer = HeightfieldLayer::createEmpty(size, 32767);

	const height_t* baseHeights = m_baseLayer->getHeights();
	const height_t* offsetHeights = m_offsetLayer->getHeights();

	height_t* mergedHeights = mergedLayer->getHeights();

	for (uint32_t i = 0; i < size * size; ++i)
		mergedHeights[i] = baseHeights[i] + int32_t(offsetHeights[i]) - 32767;

	return mergedLayer;
}

	}
}
