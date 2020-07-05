-- test script for the "human" in the scene
name = "human"
position = {x=0, y=1, z =0}
active = false

acc = 0.001
x = 0


function setup_blendingtree()

    engine.print("setup bending tree")

    local animator = gameobject:CreateComponent("Animator")
    -- skeleton
    skeleton = engine.level:GetSkeleton(0)
    
    -- animations
    animation_walk = engine.level:GetAnimation(0);
    animation_left_turn = engine.level:GetAnimation(1);
    animation_right_turn = engine.level:GetAnimation(2);

    -- blending nodes
    walk_node = gameobject:CreateObject("BlendingNode")
    left_node = gameobject:CreateObject("BlendingNode")
    right_node = gameobject:CreateObject("BlendingNode")
    bend_node3 = gameobject:CreateObject("BlendingNode3")
    -- set animations
    walk_node:SetAnimationClip(animation_walk, 0)
    left_node:SetAnimationClip(animation_left_turn, 0)
    right_node:SetAnimationClip(animation_right_turn, 0)
    -- setup blending tree
    bend_node3:SetNode(0, left_node)
    bend_node3:SetNode(1, walk_node)
    bend_node3:SetNode(2, right_node)
    bend_node3:SetParameter("x", 0)

    -- add node
    animator:SetBlendingNode(bend_node3)
    animator:SetSkeleton(skeleton)

    -- add animator component
    gameobject:AddComponent(animator)
end


function on_start()
    -- subscribe to event id 1025
    gameobject:Subscribe(1025, "on_test")
    gameobject:Subscribe(1026, "test_create")
    position = gameobject:GetTranslation()
    engine.print("on_start")

    -- setup blending node tree
    setup_blendingtree()

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

    -- right 
    local right = engine.input:GetAction(4)
    local left = engine.input:GetAction(3)
    if (right ~= 0)
    then
        x = x + acc * ms
    end
    -- left
    if (left ~= 0)
    then
        x = x + -acc * ms
    end
    -- always walk forward
    if (right == 0 and left == 0)
    then
        if (x > 0)
        then
            x = x + -acc * ms
        else 
            x = x + acc * ms
        end
    end
    blending_node:SetParameter("x", x)
end
