#include "Drawing/Image.h"
#include "Spark/BitmapImage.h"
#include "Spark/Editor/ConvertImage.h"
#include "Spark/Frame.h"
#include "Spark/Movie.h"
#include "Spark/Shape.h"
#include "Spark/Sprite.h"

namespace traktor::spark
{

Ref< Movie > convertImage(const db::Instance* sourceInstance, IStream* sourceStream, const std::wstring& extension)
{
	Ref< drawing::Image > image = drawing::Image::load(sourceStream, extension);
	if (!image)
		return nullptr;

	// Create a single frame and place shape.
	Ref< Frame > frame = new Frame();

	Frame::PlaceObject p;
	p.hasFlags = Frame::PfHasCharacterId;
	p.depth = 1;
	p.characterId = 1;
	frame->placeObject(p);

	// Create sprite and add frame.
	Ref< Sprite > sprite = new Sprite();
	sprite->addFrame(frame);

	// Create quad shape and fill with bitmap.
	Ref< Shape > shape = new Shape();
	shape->create(1, image->getWidth() * 20, image->getHeight() * 20);

	// Setup dictionary.
	Ref< Movie > movie = new Movie(Aabb2(Vector2(0.0f, 0.0f), Vector2(image->getWidth() * 20.0f, image->getHeight() * 20.0f)), sprite);
	movie->defineBitmap(1, new BitmapImage(image));
	movie->defineCharacter(1, shape);
	return movie;
}

}
