
function transform0(current)
{
	var type = current.getAttribute("type", "").getValue();
	if (type == "traktor.input.InCombine")
	{
		var xsource0 = current.getSingle("source/item[0]");
		var xsource1 = current.getSingle("source/item[1]");
		var xvalueMul0 = current.getSingle("valueMul/item[0]");
		var xvalueMul1 = current.getSingle("valueMul/item[1]");
		var xvalueAdd0 = current.getSingle("valueAdd/item[0]");
		var xvalueAdd1 = current.getSingle("valueAdd/item[1]");
		
		xsource0.setName("source");
		xvalueMul0.setName("mul");
		xvalueAdd0.setName("add");
		
		xsource1.setName("source");
		xvalueMul1.setName("mul");
		xvalueAdd1.setName("add");
		
		var xentries = new traktor.xml.Element("entries");

		var xitem0 = new traktor.xml.Element("item");
		xitem0.addChild(xsource0);
		xitem0.addChild(xvalueMul0);
		xitem0.addChild(xvalueAdd0);
		xentries.addChild(xitem0);

		var xitem1 = new traktor.xml.Element("item");
		xitem1.addChild(xsource1);
		xitem1.addChild(xvalueMul1);
		xitem1.addChild(xvalueAdd1);
		xentries.addChild(xitem1);
		
		current.removeAllChildren();
		current.addChild(xentries);
	}
	else if (type == "traktor.input.InCombine3")
	{
		var xsource0 = current.getSingle("source/item[0]");
		var xsource1 = current.getSingle("source/item[1]");
		var xsource2 = current.getSingle("source/item[2]");
		var xvalueMul0 = current.getSingle("valueMul/item[0]");
		var xvalueMul1 = current.getSingle("valueMul/item[1]");
		var xvalueMul2 = current.getSingle("valueMul/item[2]");
		var xvalueAdd0 = current.getSingle("valueAdd/item[0]");
		var xvalueAdd1 = current.getSingle("valueAdd/item[1]");
		var xvalueAdd2 = current.getSingle("valueAdd/item[2]");
		
		xsource0.setName("source");
		xvalueMul0.setName("mul");
		xvalueAdd0.setName("add");
		
		xsource1.setName("source");
		xvalueMul1.setName("mul");
		xvalueAdd1.setName("add");

		xsource2.setName("source");
		xvalueMul2.setName("mul");
		xvalueAdd2.setName("add");

		var xentries = new traktor.xml.Element("entries");

		var xitem0 = new traktor.xml.Element("item");
		xitem0.addChild(xsource0);
		xitem0.addChild(xvalueMul0);
		xitem0.addChild(xvalueAdd0);
		xentries.addChild(xitem0);

		var xitem1 = new traktor.xml.Element("item");
		xitem1.addChild(xsource1);
		xitem1.addChild(xvalueMul1);
		xitem1.addChild(xvalueAdd1);
		xentries.addChild(xitem1);
		
		var xitem2 = new traktor.xml.Element("item");
		xitem2.addChild(xsource2);
		xitem2.addChild(xvalueMul2);
		xitem2.addChild(xvalueAdd2);
		xentries.addChild(xitem2);		
		
		current.setAttribute("type", "traktor.input.InCombine");
		current.removeAllChildren();
		current.addChild(xentries);
	}
}
