import Standard;
import Traktor.Core;
import Traktor.Resource;
import Traktor.Flash;
import Traktor.Scene;

namespace parade
{

class StageData
{
	parade.LayerData[] layers;
}

class LayerData
{
	std.string name;
}

class FlashLayerData : parade.LayerData
{
	traktor.resource.Proxy< traktor.flash.FlashMovie > movie;
	traktor.Ref< parade.LayerData >[] children;
	traktor.Ref< parade.LayerData >[4] children;
	std.vector< traktor.Ref< parade.LayerData > > test;
}

class SceneLayerData : parade.LayerData
{
	traktor.resource.Proxy< traktor.scene.Scene > scene;
}

}
