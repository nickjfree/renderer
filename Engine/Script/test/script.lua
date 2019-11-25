-- test script for the "human" in the scene
name = "human"
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
	--g:Destroy()
	--engine.collectgarbage()	
end


function get_blendnode()
	-- get blending node

	local animator = gameobject:GetComponent("Animator")
	-- engine.print(gameobject:GetName())
	local node = animator:GetBlendingNode()
	return node
end


function update(ms)

    -- test_create()

	if (active ~= true)
	then
		return
	end

	local blending_node = get_blendnode()

	-- strife 
	local p = engine.input:GetAction(4)
	local n = engine.input:GetAction(3)
	if (p ~= 0)
	then
		blending_node:SetParameter("x", 0.5)
	end
	-- always walk forward
	if (p == 0 and n == 0)
	then
		blending_node:SetParameter("x", 0)
	end
end
