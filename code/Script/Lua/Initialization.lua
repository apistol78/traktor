-------------------------------------------------------------------------------
-------------------------------------------------------------------------------
--
-- Initialization script
--
-------------------------------------------------------------------------------
-------------------------------------------------------------------------------

-- Get name of instance, read "__name" member if available.
local function nameof(v)
	if v == nil then return "<nil>" end
	return rawget(v, "__name") or "<unnamed>"
end

-- Import scope into global namespace.
function import(scope)
	for n,v in pairs(scope) do
		_G[n] = v
	end
end

-- Create class.
function class(name, super)
	local cl = {}
	cl = {}

--	print("** class, name = \"" .. name .. "\", super = \"" .. nameof(super) .. "\" **")

	-- Flatten inheritance; copy tables.
	if super ~= nil then
		for k, v in pairs(super) do
--			print("** " .. name .. "[" .. k .. "] = " .. tostring(v) .. " " .. type(v))
			if type(v) == "table" then
				local c = {}
				for kk, vv in pairs(v) do c[kk] = vv end
				cl[k] = c
			else
				cl[k] = v
			end
		end
	else
		cl.__setters = {}
		cl.__getters = {}
	end

	-- Ensure these are written after "flatten".
	cl.__name = "[" .. name .. "]"
	cl.__super = super

	setmetatable(cl, {
		__call = function(cl, ...)
--			print("** " .. name .. ".__call **")
		
			-- Allocate object.
			local o
			local alloc = rawget(cl, "__alloc")
			if alloc ~= nil then o = alloc(...) else o = {} end
--			assert (o ~= nil)

			-- Setup object.
			o.__name = "[" .. name .. " instance]"
			setmetatable(o, cl)

			-- Invoke constructor.
			local new = rawget(cl, "new")
			if new ~= nil then new(o, ...) end

			return o
		end
	})

	cl.__index = function(instance, member)
--		print("** " .. name .. ".__index, instance = \"" .. nameof(instance) .. "\", member = \"" .. member .. "\" **")

		local m = rawget(cl, member)
		if m ~= nil then return m end

		local getters = rawget(cl, "__getters")
--		assert (getters ~= nil)

		local gpfn = rawget(getters, member)
		if gpfn ~= nil then return gpfn(instance) end

		local unknown = rawget(cl, "__unknown")
		if unknown ~= nil then return function(...) return unknown(member, ...) end end

		return nil
	end

	cl.__newindex = function(instance, member, value)
--	 	print("** " .. name .. ".__newindex, instance = \"" .. nameof(instance) .. "\", member = \"" .. member .. "\", value = \"" .. value .. "\" **")

	 	local setters = rawget(cl, "__setters")
	 	assert (setters ~= nil)

	 	local spfn = rawget(setters, member)
	 	if spfn ~= nil then spfn(instance, value); return end

 		rawset(instance, member, value)
	end

	return cl
end

-- Check if object "is a" class.
function isa(obj, cl)
	local obj_cl = getmetatable(obj)
	while obj_cl ~= nil do
		if cl == obj_cl then return true end
		obj_cl = obj_cl.__super
	end
	return false
end
