
name = "test component"
id = 1337
speed = 5.0
position = {x=0, y=1, z =0}
active = false

function on_start()
	-- subscribe to event id 1025
	gameobject:Subscribe(1025, "on_test")
	engine.print("on_start")
end


function on_test()
	engine.print("on_test")
	active = not active
end

function update(ms)
    local delta = ms/1000.0 * speed;
	position.x = position.x + delta;
	if position.x > 5 then
		speed = -speed
	elseif position.x < -5 then
		speed = -speed
	end
	if active then
		gameobject:SetTranslation(position)
	end
end
