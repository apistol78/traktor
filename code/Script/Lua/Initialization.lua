-------------------------------------------------------------------------------
-------------------------------------------------------------------------------
--
-- Initialization script
--
-------------------------------------------------------------------------------
-------------------------------------------------------------------------------

-- Return true if table is a class.
function isclass(t)
	local tp = rawget(t, "__type")
	if tp ~= nil then
		return tp == "class"
	else
		return false
	end
end

-- Return true if table is an instance.
function isinstance(t)
	local tp = rawget(t, "__type")
	if tp ~= nil then
		return tp == "instance"
	else
		return false
	end
end

-- Get name of instance, read "__name" member if available.
function nameof(v)
	if v == nil then return "<nil>" end
	return rawget(v, "__name") or "<unnamed>"
end

-- Get super class of class.
function superof(c)
	if c == nil then return nil end
	return rawget(c, "__super")
end

-- Import scope into global namespace.
function import(scope)
	if scope ~= nil and type(scope) == "table" then
		for n,v in pairs(scope) do
			_G[n] = v
		end
	end
end

-- Create class.
function class(name, super)
	local cl = {}
	cl = {}

	-- Flatten inheritance; copy tables.
	if super ~= nil then
		for k, v in pairs(super) do
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
	cl.__type = "class"
	cl.__super = super

	setmetatable(cl, {
		__call = function(cl, ...)
			-- Allocate object.
			local o
			local alloc = rawget(cl, "__alloc")
			if alloc ~= nil then o = alloc(...) else o = {} end

			-- Setup object.
			o.__name = "[" .. name .. " instance]"
			o.__type = "instance"
			setmetatable(o, cl)

			-- Invoke constructor.
			local new = rawget(cl, "new")
			if new ~= nil then new(o, ...) end

			return o
		end
	})

	cl.__index = function(instance, member)
		local m = rawget(cl, member)
		if m ~= nil then return m end

		local getters = rawget(cl, "__getters")

		local gpfn = rawget(getters, member)
		if gpfn ~= nil then return gpfn(instance) end

		local unknown = rawget(cl, "__unknown")
		if unknown ~= nil then return function(...) return unknown(member, ...) end end

		return nil
	end

	cl.__newindex = function(instance, member, value)
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
		obj_cl = rawget(obj_cl, "__super")
	end
	return false
end
