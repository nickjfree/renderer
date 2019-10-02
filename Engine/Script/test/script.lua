-- test script for the "usb driver" in the scene
name = "adata"
id = 1337
speed = 0.01
rspeed = 0.1 * (3.1415926 / 180.0)
position = {x=0, y=1, z =0}
active = false

function on_start()
	-- subscribe to event id 1025
	gameobject:Subscribe(1025, "on_test")
	gameobject:Subscribe(1026, "test_create")
	position = gameobject:GetTranslation()
	--engine.print("on_start")
end


function on_test()
	engine.print("on_test")
	active = not active
end



function test_create()
	md=engine.level:GetModel(1)
	mt=engine.level:GetMaterial(1)
	g = engine.scene:CreateGameObject("test")
	-- add renderer
	r = engine.scene:CreateComponent("Renderer")
	r:SetModel(md)
	r:SetMaterial(mt)
	p = gameobject:GetTranslation()
	g:SetTranslation(p)
	g:AddComponent(r)
	-- add physics
	py = engine.scene:CreateComponent("PhysicsObject")
	py:CreateShapeFromModel(md)
	g:AddComponent(py)
	-- engine.print(g:GetObjectId())
	g:Destroy()
	engine.collectgarbage()	
end



function update(ms)

    test_create()

	if (active ~= true)
	then
		return
	end
	--test_create()
    local delta = ms * speed
	local rad = rspeed * ms
	-- walk 
	local p = engine.input:GetAction(1)
	local n = engine.input:GetAction(2)
	if (p ~= 0)
	then
		gameobject:Walk(delta)
	end
	if (n ~= 0)
	then
		gameobject:Walk(-delta)
	end
	p = 0
	n = 0
	-- strife 
	local p = engine.input:GetAction(4)
	local n = engine.input:GetAction(3)
	if (p ~= 0)
	then
		gameobject:Strife(delta)
	end
	if (n ~= 0)
	then
		gameobject:Strife(-delta)
	end
	p = 0
	n = 0
	-- ascend 
	local p = engine.input:GetAction(5)
	local n = engine.input:GetAction(6)
	if (p ~= 0)
	then
		gameobject:Ascend(delta)
	end
	if (n ~= 0)
	then
		gameobject:Ascend(-delta)
	end
	p = 0
	n = 0
	-- pitch 
	local p = engine.input:GetAction(9)
	local n = engine.input:GetAction(10)
	if (p ~= 0)
	then
		gameobject:Pitch(rad)
	end
	if (n ~= 0)
	then
		gameobject:Pitch(-rad)
	end
	p = 0
	n = 0
	-- yaw 
	local p = engine.input:GetAction(8)
	local n = engine.input:GetAction(7)
	if (p ~= 0)
	then
		gameobject:Yaw(rad)
	end
	if (n ~= 0)
	then
		gameobject:Yaw(-rad)
	end
	p = 0
	n = 0
	-- roll 
	local p = engine.input:GetAction(18)
	local n = engine.input:GetAction(19)
	if (p ~= 0)
	then
		gameobject:Roll(rad)
	end
	if (n ~= 0)
	then
		gameobject:Roll(-rad)
	end
	p = 0
	n = 0
end
