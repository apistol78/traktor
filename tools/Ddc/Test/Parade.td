import Standard;
import Traktor.Core;
import Traktor.Resource;
import Traktor.Flash;
import Traktor.Scene;

namespace parade
{

class StageData
{
	LayerData[] layers;
}

class LayerData
{
	string name;
}

class FlashLayerData : LayerData
{
	traktor.resource.Proxy< traktor.flash.FlashMovie > movie;
	traktor.Ref< LayerData >[] children;
	traktor.Ref< LayerData >[4] children;
	std.vector< traktor.Ref< LayerData > > test;
}

class SceneLayerData : LayerData
{
	traktor.resource.Proxy< traktor.scene.Scene > scene;
}

}
