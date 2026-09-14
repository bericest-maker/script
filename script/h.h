local Players = game:GetService("Players")
local RS = game:GetService("ReplicatedStorage")
local LP = Players.LocalPlayer
local PG = LP:WaitForChild("PlayerGui")
local WS = game:GetService("Workspace")
local PurchRS = RS.Shared.Resources.VendorResources.Remotes.PurchaseStructure

local CFG = {REFRESH = 0.5, CLEAN_INT = 2, MAX_HP = 300000, BUY_DEL = 0.08, BOSS_MOVE = 15, TP_BACK = 0.5}
local S = {mythic = false, boss = false, clean = false}
local stats = {m = 0, b = 0, c = 0}

local gui = Instance.new("ScreenGui")
gui.Name = "AB"
gui.ResetOnSpawn = false
gui.Parent = PG

local fr = Instance.new("Frame")
fr.Size = UDim2.new(0, 160, 0, 80)
fr.Position = UDim2.new(0, 10, 0, 10)
fr.BackgroundColor3 = Color3.fromRGB(20, 20, 25)
fr.BorderSizePixel = 0
fr.Active = true
fr.Draggable = true
fr.Parent = gui

local info = Instance.new("TextLabel")
info.Size = UDim2.new(1, 0, 0, 16)
info.BackgroundColor3 = Color3.fromRGB(30, 30, 38)
info.BorderSizePixel = 0
info.Text = "M:0 B:0 C:0"
info.TextColor3 = Color3.fromRGB(180, 180, 180)
info.TextSize = 10
info.Font = Enum.Font.Code
info.Parent = fr

local function mkToggle(par, txt, key, y)
    local b = Instance.new("TextButton")
    b.Size = UDim2.new(1, -4, 0, 18)
    b.Position = UDim2.new(0, 2, 0, y)
    b.BackgroundColor3 = Color3.fromRGB(35, 35, 45)
    b.BorderSizePixel = 0
    b.Text = txt..": OFF"
    b.TextColor3 = Color3.fromRGB(130, 130, 130)
    b.TextSize = 10
    b.Font = Enum.Font.Code
    b.Parent = par
    b.MouseButton1Click:Connect(function()
        S[key] = not S[key]
        b.Text = txt..(S[key] and ": ON" or ": OFF")
        b.BackgroundColor3 = S[key] and Color3.fromRGB(0, 80, 40) or Color3.fromRGB(35, 35, 45)
        b.TextColor3 = S[key] and Color3.new(1,1,1) or Color3.fromRGB(130, 130, 130)
    end)
end
mkToggle(fr, "Mythic", "mythic", 18)
mkToggle(fr, "Boss", "boss", 38)
mkToggle(fr, "Clean", "clean", 58)

local function updInfo()
    info.Text = "M:"..stats.m.." B:"..stats.b.." C:"..stats.c
end

local function getRoot()
    local c = LP.Character
    return c and c:FindFirstChild("HumanoidRootPart")
end

local function tpTo(pos)
    local r = getRoot()
    if r then r.CFrame = CFrame.new(pos) end
end

local function fireP(p)
    pcall(function()
        if p and p.Parent then
            p:InputHoldBegin()
            task.wait(0.05)
            p:InputHoldEnd()
        end
    end)
end

local function findPrompts(obj)
    local t = {}
    for _, d in ipairs(obj:GetDescendants()) do
        if d:IsA("ProximityPrompt") then table.insert(t, d) end
    end
    return t
end

local function clean()
    pcall(function()
        for _, u in ipairs(WS.ActiveUnits:GetChildren()) do
            if not u.Name:match("^BOSS:") then
                local hp = u:GetAttribute("MaxHealth")
                if hp and hp < CFG.MAX_HP then
                    u:Destroy()
                    stats.c = stats.c + 1
                end
            end
        end
    end)
    pcall(function()
        for _, i in ipairs(WS.Center.Islands:GetChildren()) do
            if i.Name:match("Oil Rig") then i:Destroy() end
        end
    end)
    pcall(function()
        for _, p in ipairs(WS.Center.Islands.SideIslandProps:GetChildren()) do p:Destroy() end
    end)
    pcall(function()
        for _, p in ipairs(WS.Center.Props:GetChildren()) do p:Destroy() end
    end)
    pcall(function()
        for _, w in ipairs(WS.Components.Weather:GetChildren()) do w:Destroy() end
    end)
    pcall(function()
        local af = WS.Center:FindFirstChild("AreaInFront")
        if af then for _, c in ipairs(af:GetChildren()) do c:Destroy() end end
    end)
    pcall(function()
        for _, pl in ipairs(WS.Plots:GetChildren()) do
            local bp = pl:FindFirstChild("baseplate")
            if bp then
                local pd = bp:FindFirstChild("plotDecoration")
                if pd then for _, c in ipairs(pd:GetChildren()) do c:Destroy() end end
            end
        end
    end)
    updInfo()
end

local function scanShop()
    local sf = PG:FindFirstChild("shopVendor")
    if not sf then return end
    local m = sf:FindFirstChild("main")
    if not m then return end
    local sf2 = m:FindFirstChild("shopFrame")
    if not sf2 then return end
    for _, cat in ipairs({"units","decoration","production","special"}) do
        local cf = sf2:FindFirstChild(cat)
        if cf then
            for _, item in ipairs(cf:GetChildren()) do
                local rar = item:FindFirstChild("rarity")
                if rar and (rar:IsA("TextLabel") or rar:IsA("TextButton")) then
                    if rar.Text:upper():find("MYTHIC") then
                        local stk = item:FindFirstChild("stockFrame")
                        local sa = stk and stk:FindFirstChild("stockAmount")
                        local stock = sa and tonumber(sa.Text) or 0
                        if stock > 0 then
                            for _ = 1, stock do
                                pcall(function() PurchRS:FireServer(item.Name) end)
                                stats.m = stats.m + 1
                                updInfo()
                                task.wait(CFG.BUY_DEL)
                            end
                        end
                    end
                end
            end
        end
    end
end

local cpCache = {}
local function initCP()
    pcall(function()
        local cp = WS.Components.ControlPoints
        if not cp then return end
        for _, p in ipairs(cp:GetChildren()) do
            local ix = p:FindFirstChild("interact")
            if ix then
                local g = ix:FindFirstChild("GroundCaptureTarget") or ix:FindFirstChild("AirCaptureTarget") or ix:FindFirstChild("NavalCaptureTarget")
                if g then table.insert(cpCache, {name = p.Name, obj = g}) end
            end
        end
    end)
end

local lastBossPos = nil
local function huntBoss()
    local au = WS:FindFirstChild("ActiveUnits")
    if not au then return end
    for _, u in ipairs(au:GetChildren()) do
        if u.Name:match("^BOSS:") then
            local root = u:FindFirstChild("root") or u:FindFirstChild("HumanoidRootPart") or u:FindFirstChild("Torso")
            if not root then
                for _, d in ipairs(u:GetDescendants()) do
                    if d:IsA("BasePart") then root = d; break end
                end
            end
            if root then
                local cur = root.Position
                if lastBossPos and (cur - lastBossPos).Magnitude < CFG.BOSS_MOVE then return end
                lastBossPos = cur

                local best, bestD = nil, math.huge
                for _, cp in ipairs(cpCache) do
                    local d = (cp.obj.Position - cur).Magnitude
                    if d < bestD then bestD = d; best = cp end
                end
                if not best then return end

                stats.b = stats.b + 1
                updInfo()
                local savedCF = getRoot() and getRoot().CFrame
                tpTo(best.obj.Position)
                for _, p in ipairs(findPrompts(best.obj)) do fireP(p) end
                task.wait(CFG.TP_BACK)
                if savedCF then local r = getRoot(); if r then r.CFrame = savedCF end end
            end
            break
        end
    end
end

initCP()
clean()

task.spawn(function()
    while gui.Parent do
        if S.mythic then pcall(scanShop) end
        if S.boss then pcall(huntBoss) end
        task.wait(CFG.REFRESH)
    end
end)

task.spawn(function()
    while gui.Parent do
        if S.clean then pcall(clean) end
        task.wait(CFG.CLEAN_INT)
    end
end)
