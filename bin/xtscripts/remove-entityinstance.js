

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
		type == "traktor.theater.TrackData"
	)
	{
		var instance = current.getChildElementByName("instance");
		if (instance == null)
			return;
		
		instance.setName("entityData");
	}
	
	if (
		type == "traktor.world.GroupEntityData" ||
		type == "traktor.mesh.CompositeMeshEntityData"
	)
	{
		var instances = current.getChildElementByName("instances");
		if (instances == null)
			return;
		
		instances.setName("entityData");
	}
	
}
