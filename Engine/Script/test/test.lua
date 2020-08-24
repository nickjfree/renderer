print("initialize scripting engine")

-- weak table to stote all objects
objects = {}
setmetatable(objects, { __mode = 'v' })

-- table only to store gameobjects or exported tables
entities = {}

-- scripts to store scripts template
scripts = {}

-- metatable for destroyed objects
destroyed_mt = {}

destroyed_mt.__index = function (table, key)
	error("attemped to operate on destroyed gameobject!!!")
end

function clone(source, target)
    for k, v in pairs(source) do 
        if type(v) == "table" then
            t = {}
            clone(v, t)
            target[k] = v
        else
            target[k] = v
        end
    end
end

function reset()
    collectgarbage()
    loader:UnloadLevel("Level\\levels\\desktop.level\\0")
end

function control()
    scene:SendEvent(1025)
end


function setEnv(intensity)
    lightObject = scene:GetGameObject("LightProb")
    light = lightObject:GetComponent("Light")
    light:SetIntensity(intensity)
end