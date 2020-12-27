-- test script for the "human" in the scene
name = "light"
position = {x=0, y=1, z =0}
active = false

acc = 0.001
x = 0
speed = 0.005



function on_start()
    -- light
    engine.print("light: on_start")
end


function update(ms)

    -- test_create()
    local delta = ms * speed
    gameobject:Strife(delta)

    local position = gameobject:GetTranslation()
    if position.x > 100 or  position.x < -10 then
        speed = -speed
    end
end
