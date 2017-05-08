
name = "test component"
id = 1337
speed = 5.0
position = {x=0, y=1, z =0}

function update(ms)
    local delta = ms/1000.0 * speed;
	position.x = position.x + delta;
	if position.x > 5 then
		speed = -speed
	elseif position.x < -5 then
		speed = -speed
	end
	gameobject:SetTranslation(position)
	engine.print("i moved")
end
