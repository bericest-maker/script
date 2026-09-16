local Players = game:GetService("Players")
local LocalPlayer = Players.LocalPlayer
local PlayerGui = LocalPlayer:WaitForChild("PlayerGui")
local Workspace = game:GetService("Workspace")
local RS = game:GetService("ReplicatedStorage")

local CONFIG = {REFRESH_RATE=1, CLEAN_INTERVAL=2, MAX_HEALTH_DELETE=300000, AUTO_JUMP=true, TP_SPEED=0.3}

local PurchaseRemote = RS:WaitForChild("Shared"):WaitForChild("Resources"):WaitForChild("VendorResources"):WaitForChild("Remotes"):WaitForChild("PurchaseStructure")

local function cleanupWorkspace()
    pcall(function() for _,u in ipairs(Workspace.ActiveUnits:GetChildren()) do if not u:GetAttribute("BossName") then local mh=u:GetAttribute("MaxHealth") if mh and mh<CONFIG.MAX_HEALTH_DELETE then u:Destroy() end end end end)
    pcall(function() for _,i in ipairs(Workspace.Center.Islands:GetChildren()) do if i.Name:match("Oil Rig") then i:Destroy() end end end)
    pcall(function() for _,p in ipairs(Workspace.Center.Islands.SideIslandProps:GetChildren()) do p:Destroy() end end)
    pcall(function() for _,p in ipairs(Workspace.Center.Props:GetChildren()) do p:Destroy() end end)
    pcall(function() for _,w in ipairs(Workspace.Components.Weather:GetChildren()) do w:Destroy() end end)
    pcall(function() local af=Workspace.Center:FindFirstChild("AreaInFront") if af then for _,c in ipairs(af:GetChildren()) do c:Destroy() end end end)
    pcall(function() for _,p in ipairs(Workspace.Plots:GetChildren()) do local b=p:FindFirstChild("baseplate") if b then local d=b:FindFirstChild("plotDecoration") if d then for _,c in ipairs(d:GetChildren()) do c:Destroy() end end end end end)
end

local consoleLines={"","",""}
local consoleLabels={}
local function updateConsole() for i=1,3 do if consoleLabels[i] then consoleLabels[i].Text=consoleLines[i] end end end
local function logConsole(m) table.remove(consoleLines,1) table.insert(consoleLines,m) updateConsole() end

local screenGui=Instance.new("ScreenGui")
screenGui.Name="A"
screenGui.ResetOnSpawn=false
screenGui.Parent=PlayerGui

local mainFrame=Instance.new("Frame")
mainFrame.Size=UDim2.new(0,200,0,120)
mainFrame.Position=UDim2.new(0,10,0.5,-60)
mainFrame.BackgroundColor3=Color3.fromRGB(20,20,20)
mainFrame.BorderSizePixel=0
mainFrame.Active=true
mainFrame.Draggable=true
mainFrame.Parent=screenGui

local title=Instance.new("TextLabel")
title.Size=UDim2.new(1,0,0,20)
title.BackgroundColor3=Color3.fromRGB(30,30,30)
title.BorderSizePixel=0
title.Text="A"
title.TextColor3=Color3.fromRGB(0,255,0)
title.TextSize=12
title.Font=Enum.Font.Code
title.Parent=mainFrame

for i=1,3 do
    local l=Instance.new("TextLabel")
    l.Size=UDim2.new(1,-8,0,16)
    l.Position=UDim2.new(0,4,0,22+(i-1)*18)
    l.BackgroundTransparency=1
    l.Text=""
    l.TextColor3=Color3.fromRGB(200,200,200)
    l.TextSize=11
    l.Font=Enum.Font.Code
    l.TextXAlignment=Enum.TextXAlignment.Left
    l.TextTruncate=Enum.TextTruncate.AtEnd
    l.Parent=mainFrame
    consoleLabels[i]=l
end

local states={m=false,b=false,c=false}
local stats={Mythics=0,Bosses=0,Cleaned=0}

local function createToggle(t,k,x)
    local b=Instance.new("TextButton")
    b.Size=UDim2.new(0.3,-2,0,20)
    b.Position=UDim2.new(x,0,0,80)
    b.BackgroundColor3=Color3.fromRGB(40,40,40)
    b.BorderSizePixel=0
    b.Text=t
    b.TextColor3=Color3.fromRGB(150,150,150)
    b.TextSize=10
    b.Font=Enum.Font.Code
    b.Parent=mainFrame
    b.MouseButton1Click:Connect(function()
        states[k]=not states[k]
        b.BackgroundColor3=states[k] and Color3.fromRGB(0,100,50) or Color3.fromRGB(40,40,40)
        b.TextColor3=states[k] and Color3.fromRGB(255,255,255) or Color3.fromRGB(150,150,150)
    end)
end

createToggle("M","m",0)
createToggle("B","b",0.35)
createToggle("C","c",0.7)

local statsLabel=Instance.new("TextLabel")
statsLabel.Size=UDim2.new(1,-8,0,14)
statsLabel.Position=UDim2.new(0,4,0,104)
statsLabel.BackgroundTransparency=1
statsLabel.Text="M:0 B:0 C:0"
statsLabel.TextColor3=Color3.fromRGB(100,200,255)
statsLabel.TextSize=10
statsLabel.Font=Enum.Font.Code
statsLabel.TextXAlignment=Enum.TextXAlignment.Left
statsLabel.Parent=mainFrame

local function updateStats() statsLabel.Text=string.format("M:%d B:%d C:%d",stats.Mythics,stats.Bosses,stats.Cleaned) end

local closeBtn=Instance.new("TextButton")
closeBtn.Size=UDim2.new(0,16,0,16)
closeBtn.Position=UDim2.new(1,-18,0,2)
closeBtn.BackgroundColor3=Color3.fromRGB(150,30,30)
closeBtn.BorderSizePixel=0
closeBtn.Text="X"
closeBtn.TextColor3=Color3.fromRGB(255,255,255)
closeBtn.TextSize=10
closeBtn.Font=Enum.Font.Code
closeBtn.Parent=mainFrame
closeBtn.MouseButton1Click:Connect(function() screenGui:Destroy() end)

local function getRoot() local c=LocalPlayer.Character return c and c:FindFirstChild("HumanoidRootPart") end

local function autoJump()
    if not CONFIG.AUTO_JUMP then return end
    local c=LocalPlayer.Character
    if c then local h=c:FindFirstChild("Humanoid") if h and h.FloorMaterial~=Enum.Material.Air then h.Jump=true end end
end

local function moveTo(tp,t)
    local r=getRoot() if not r then return false end
    t=t or 10 local s=tick()
    while tick()-s<t do
        r=getRoot() if not r then return false end
        if (r.Position-tp).Magnitude<5 then return true end
        r.CFrame=CFrame.new(r.Position,Vector3.new(tp.X,r.Position.Y,tp.Z))
        r.Velocity=Vector3.new(0,r.Velocity.Y,0)
        local d=(tp-r.Position).Unit
        r.Velocity=Vector3.new(d.X*50,r.Velocity.Y,d.Z*50)
        autoJump()
        task.wait(0.05)
    end
    return false
end

local function firePrompt(p)
    pcall(function() if p and p.Parent then p:InputHoldBegin() task.wait(0.1) p:InputHoldEnd() end end)
end

local shopCategories={"units","decoration","production","special"}
local buying=false

local function scanShop()
    if buying then return end
    local sf=PlayerGui:FindFirstChild("shopVendor") if not sf then return end
    local m=sf:FindFirstChild("main") if not m then return end
    local sf2=m:FindFirstChild("shopFrame") if not sf2 then return end
    for _,cat in ipairs(shopCategories) do
        local cf=sf2:FindFirstChild(cat)
        if cf then
            for _,item in ipairs(cf:GetChildren()) do
                local r=item:FindFirstChild("rarity")
                if r and (r:IsA("TextLabel") or r:IsA("TextButton")) and r.Text:upper():find("MYTHIC") then
                    local sf3=item:FindFirstChild("stockFrame")
                    local sa=sf3 and sf3:FindFirstChild("stockAmount")
                    local stock=sa and tonumber(sa.Text) or 0
                    if stock>0 then
                        buying=true
                        logConsole('Found mythic "'..item.Name..'"')
                        task.wait(0.1)
                        local bc=0
                        while bc<stock do
                            PurchaseRemote:FireServer(item.Name)
                            bc=bc+1
                            stats.Mythics=stats.Mythics+1
                            logConsole('Bought '..bc..' "'..item.Name..'"')
                            updateStats()
                            task.wait(0.08)
                            local ns=sa and tonumber(sa.Text) or 0
                            if ns<=0 then break end
                        end
                        buying=false
                    end
                end
            end
        end
    end
end

local capturePoints={
    {name="Center",air=nil,ground=nil,naval=nil},
    {name="1",air=nil,ground=nil,naval=nil},
    {name="5",air=nil,ground=nil,naval=nil},
}

local function initCapturePoints()
    pcall(function()
        local cp=Workspace.Components.ControlPoints
        if cp then
            if cp:FindFirstChild("Center") then local c=cp.Center if c:FindFirstChild("interact") then capturePoints[1].air=c.interact:FindFirstChild("AirCaptureTarget") capturePoints[1].ground=c.interact:FindFirstChild("GroundCaptureTarget") end end
            if cp:FindFirstChild("1") then local p=cp["1"] if p:FindFirstChild("interact") then capturePoints[2].air=p.interact:FindFirstChild("AirCaptureTarget") capturePoints[2].ground=p.interact:FindFirstChild("GroundCaptureTarget") end end
            if cp:FindFirstChild("8") then local p=cp["8"] if p:FindFirstChild("interact") then capturePoints[3].air=p.interact:FindFirstChild("AirCaptureTarget") capturePoints[3].naval=p.interact:FindFirstChild("NavalCaptureTarget") end end
        end
    end)
end

local huntingBoss=false
local lastBossPos=nil

local function getClosestPoint(bp)
    local cl,cd=nil,math.huge
    for _,p in ipairs(capturePoints) do
        local t=p.ground or p.air or p.naval
        if t then
            local d=(t.Position-bp).Magnitude
            if d<cd then cd=d cl=p end
        end
    end
    return cl
end

local function huntBoss()
    if huntingBoss then return end
    local au=Workspace:FindFirstChild("ActiveUnits") if not au then return end
    local bossFound=false
    for _,u in ipairs(au:GetChildren()) do
        local bn=u:GetAttribute("BossName")
        if bn then
            bossFound=true
            huntingBoss=true
            local br=u:FindFirstChild("root") or u:FindFirstChild("HumanoidRootPart") or u:FindFirstChild("Torso")
            if not br then for _,c in ipairs(u:GetDescendants()) do if c:IsA("BasePart") then br=c break end end end
            if br then
                local wc=br:GetAttribute("WorldCFrame")
                local bp=wc and Vector3.new(wc.X,wc.Y,wc.Z) or br.Position
                if lastBossPos and (bp-lastBossPos).Magnitude>10 then
                    local cp=getClosestPoint(bp)
                    if cp then
                        local tt=cp.ground and "ground" or (cp.naval and "naval" or "air")
                        logConsole('Boss moved, redirecting to "'..cp.name..'" ('..tt..')')
                        local r=getRoot()
                        if r then
                            local oc=r.CFrame
                            local pt=cp.ground or cp.naval or cp.air
                            if pt then r.CFrame=pt.CFrame task.wait(CONFIG.TP_SPEED) firePrompt(pt) end
                            if cp.air and cp.air~=pt then r.CFrame=cp.air.CFrame task.wait(CONFIG.TP_SPEED) firePrompt(cp.air) end
                            r.CFrame=oc
                        end
                    end
                elseif not lastBossPos then
                    stats.Bosses=stats.Bosses+1
                    updateStats()
                    logConsole('Found boss "'..bn..'"')
                end
                lastBossPos=bp
            end
            huntingBoss=false
            break
        end
    end
    if not bossFound and lastBossPos then
        lastBossPos=nil
        logConsole("Boss ended, returning to Center")
        local r=getRoot()
        if r and capturePoints[1].ground then
            local oc=r.CFrame
            r.CFrame=capturePoints[1].ground.CFrame
            task.wait(CONFIG.TP_SPEED)
            firePrompt(capturePoints[1].ground)
            if capturePoints[1].air then r.CFrame=capturePoints[1].air.CFrame task.wait(CONFIG.TP_SPEED) firePrompt(capturePoints[1].air) end
            r.CFrame=oc
        end
    end
end

local function cleanActiveUnits()
    local au=Workspace:FindFirstChild("ActiveUnits") if not au then return end
    for _,u in ipairs(au:GetChildren()) do
        if not u:GetAttribute("BossName") then
            local mh=u:GetAttribute("MaxHealth")
            if mh and mh<CONFIG.MAX_HEALTH_DELETE then
                pcall(function() u:Destroy() end)
                stats.Cleaned=stats.Cleaned+1
            end
        end
    end
    updateStats()
end

initCapturePoints()
cleanupWorkspace()
logConsole("Ready")

task.spawn(function() while screenGui.Parent do if states.m then pcall(scanShop) end task.wait(CONFIG.REFRESH_RATE) end end)
task.spawn(function() while screenGui.Parent do if states.b then pcall(huntBoss) end task.wait(CONFIG.REFRESH_RATE) end end)
task.spawn(function() while screenGui.Parent do if states.c then pcall(cleanActiveUnits) end task.wait(CONFIG.CLEAN_INTERVAL) end end)

local s={3,5,10,60}
local i=1
local g=gethui()
for _,v in pairs(g:GetChildren()) do if v.Name=="FPSGui" then v:Destroy() end end
local sg=Instance.new("ScreenGui",g)
sg.Name="FPSGui"
sg.ResetOnSpawn=false
local b=Instance.new("TextButton",sg)
b.Size=UDim2.new(0,80,0,25)
b.Position=UDim2.new(1,-90,0,10)
b.BackgroundColor3=Color3.fromRGB(25,25,25)
b.BackgroundTransparency=.15
b.Text="FPS: OFF"
b.TextColor3=Color3.new(1,1,1)
b.TextSize=12
b.Font=Enum.Font.Code
b.MouseButton1Click:Connect(function()
    i=i%4+1
    local f=s[i]
    if setfpscap then setfpscap(f) end
    b.Text=f~=60 and "FPS: "..f or "FPS: OFF"
end)
