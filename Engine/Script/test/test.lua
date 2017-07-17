print("initialize scripting engine")

-- weak table to stote all objects
objects = {}
setmetatable(objects, { __mode = 'v' })

-- table only to store gameobjects
entities = {}


-- metatable for destroyed objects
destroyed_mt = {}

destroyed_mt.__index = function (table, key)
	error("attemped to operate on destroyed gameobject!!!")
end