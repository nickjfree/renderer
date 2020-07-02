md=level:GetModel(3)
mt=level:GetMaterial(14)
g = objects[5]:CreateGameObject("test")
r = scene:CreateComponent("Renderer")
r:SetModel(md)
r:SetMaterial(mt)
p = {}
p.x=0
p.y=0
p.z=5
g:SetTranslation(p)
g:AddComponent(r)
g:Destroy()
