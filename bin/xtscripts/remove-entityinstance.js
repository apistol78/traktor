
var g_instancePaths = new Array();
var g_collapseCount = 0;

function transform0(current)
{
	var type = current.getAttribute("type", "").getValue();
	if (type == "traktor.world.EntityInstance")
	{
		g_instancePaths.push([
			current.getPath(),
			null
		]);
	}
}

function transform1(current)
{
	// Collapse entity instances.
	
	var children = new Array();
	for (var child = current.getFirstChild(); child; child = child.getNextSibling())
	{
		if (child instanceof traktor.xml.Element)
			children.push(child);
	}
	
	for (var i = 0; i < children.length; ++i)
	{
		var child = children[i];
		var type = child.getAttribute("type", "").getValue();
		if (type == "traktor.world.EntityInstance")
		{
			var instanceName = child.getChildElementByName("name");
			var instanceEntityData = child.getChildElementByName("entityData");
			
			child.removeChild(instanceName);
			child.removeChild(instanceEntityData);
			
			instanceEntityData.setName(child.getName());
			instanceEntityData.insertBefore(instanceName, 0);
			
			current.insertBefore(instanceEntityData, child);
			current.removeChild(child);
			
			g_instancePaths[g_collapseCount++][1] = instanceEntityData.getPath();
		}
	}
	
	// Update references.
	
	var ref = current.getAttribute("ref", "").getValue();
	if (ref != "")
	{
		for (var i = 0; i < g_collapseCount; ++i)
		{
			if (g_instancePaths[i][0] == ref)
			{
				current.setAttribute("ref", g_instancePaths[i][1]);
				break;
			}
		}
	}

	// Rename child members.
	
	var type = current.getAttribute("type", "").getValue();
	
	if (
		type == "traktor.scene.SceneAsset" ||
		type == "traktor.theater.TrackData" ||
		type == "traktor.animation.PathEntityData" ||
		type == "traktor.physics.RigidEntityData"
	)
	{
		var instance = current.getChildElementByName("instance");
		if (instance != null)
			instance.setName("entityData");
	}
	
	if (
		type == "traktor.world.GroupEntityData" ||
		type == "traktor.world.SpatialGroupEntityData" ||
		type == "traktor.mesh.CompositeMeshEntityData" ||
		type == "traktor.physics.ArticulatedEntityData" ||
		type == "game.LayerEntityData"
	)
	{
		var instances = current.getChildElementByName("instances");
		if (instances != null)
			instances.setName("entityData");
	}
	
	// Ensure entity data have a name.
	
	if (
		type == "traktor.world.ExternalSpatialEntityData" ||
		type == "traktor.world.ExternalEntityData" ||
		type == "traktor.animation.AnimatedMeshEntityData" ||
		type == "traktor.animation.PathEntityData" ||
		type == "traktor.world.GroupEntityData" ||
		type == "traktor.world.SpatialGroupEntityData" ||
		type == "traktor.mesh.MeshEntityData" ||
		type == "traktor.mesh.CompositeMeshEntityData" ||
		type == "traktor.physics.ArticulatedEntityData" ||
		type == "traktor.physics.RigidEntityData" ||
		type == "game.CubeEntityData" ||
		type == "game.SwfMeshEntityData" ||
		type == "game.ClothEntityData" ||
		type == "game.PlayerEntityData" ||
		type == "game.LevelEntityData" ||
		type == "game.PickupEntityData"
	)
	{
		if (current.getChildElementByName("name") == null)
		{
			var name = new traktor.xml.Element("name");
			name.addChild(new traktor.xml.Text("Unnamed"));
			current.insertBefore(name, 0);
		}
	}
	
	// Update scene asset versions.
	
	if (
		type == "traktor.scene.SceneAsset"
	)
	{
		current.setAttribute("version", "3");
		
		var worldRenderSettings = current.getChildElementByName("worldRenderSettings");
		
		if (current.getChildElementByName("postProcessSettings") == null)
			current.insertAfter(new traktor.xml.Element("postProcessSettings"), worldRenderSettings);
			
		if (current.getChildElementByName("controllerData") == null)
			current.addChild(new traktor.xml.Element("controllerData"));
	}
}
