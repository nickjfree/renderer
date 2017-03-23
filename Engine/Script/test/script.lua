
name = "test component"
id = 1337
speed = 0.5
position = {x=0, y=1, z =0}

function update(ms)
    local delta = ms/1000.0 * speed;
	position.x = position.x + delta;
	gameobject:SetTranslation(position)
end
