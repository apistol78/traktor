
function createValue(oldvalue, name, newvalue)
{
	if (oldvalue != null)
		return oldvalue;
		
	var node = new traktor.xml.Element(name);
	node.addChild(new traktor.xml.Text(newvalue));
	
	return node;
}

function transform0(current)
{
	var type = current.getAttribute("type", "").getValue();
	if (type == "traktor.physics.DynamicBodyDesc")
	{
		var version = parseInt(current.getAttribute("version", "0").getValue());
		if (version < 4)
		{
			var xshape = current.getChildElementByName("shape");
			var xmass = current.getChildElementByName("mass");
			var xautoDeactivate = current.getChildElementByName("autoDeactivate");
			var xactive = current.getChildElementByName("active");
			var xlinearDamping = current.getChildElementByName("linearDamping");
			var xangularDamping = current.getChildElementByName("angularDamping");
			var xfriction = current.getChildElementByName("friction");
			
			current.removeAllChildren();
			current.addChild(xshape);
			current.addChild(createValue(xmass, "mass", "0"));
			current.addChild(createValue(xautoDeactivate, "autoDeactivate", "true"));
			current.addChild(createValue(xactive, "active", "true"));
			current.addChild(createValue(xlinearDamping, "linearDamping", "0.1"));
			current.addChild(createValue(xangularDamping, "angularDamping", "0.1"));
			current.addChild(createValue(xfriction, "friction", "0.1"));
			current.setAttribute("version", "4");
		}
	}
}
