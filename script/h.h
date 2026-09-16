local Players = game:GetService("Players")
local LocalPlayer = Players.LocalPlayer
local PlayerGui = LocalPlayer:WaitForChild("PlayerGui")
local Workspace = game:GetService("Workspace")
local RS = game:GetService("ReplicatedStorage")

local CONFIG = {
    REFRESH_RATE = 1,
    CLEAN_INTERVAL = 2,
    MAX_HEALTH_DELETE = 300000,
    AUTO_JUMP = true,
    TP_SPEED = 0.3,
}

local PurchaseRemote = RS:WaitForChild("Shared"):WaitForChild("Resources"):WaitForChild("VendorResources"):WaitForChild("Remotes"):WaitForChild("PurchaseStructure")

local function cleanupWorkspace()
    pcall(function()
        for _, unit in ipairs(Workspace.ActiveUnits:GetChildren()) do
            if not unit:GetAttribute("BossName") then
                local maxHealth = unit:GetAttribute("MaxHealth")
                if maxHealth and maxHealth < CONFIG.MAX_HEALTH_DELETE then
                    unit:Destroy()
                end
            end
        end
    end)
    pcall(function()
        for _, island in ipairs(Workspace.Center.Islands:GetChildren()) do
            if island.Name:match("Oil Rig") then island:Destroy() end
        end
    end)
    pcall(function()
        for _, prop in ipairs(Workspace.Center.Islands.SideIslandProps:GetChildren()) do
            prop:Destroy()
        end
    end)
    pcall(function()
        for _, prop in ipairs(Workspace.Center.Props:GetChildren()) do
            prop:Destroy()
        end
    end)
    pcall(function()
        for _, weather in ipairs(Workspace.Components.Weather:GetChildren()) do
            weather:Destroy()
        end
    end)
    pcall(function()
        local areaFront = Workspace.Center:FindFirstChild("AreaInFront")
        if areaFront then
            for _, child in ipairs(areaFront:GetChildren()) do child:Destroy() end
        end
    end)
    pcall(function()
        for _, plot in ipairs(Workspace.Plots:GetChildren()) do
            local baseplate = plot:FindFirstChild("baseplate")
            if baseplate then
                local plotDecoration = baseplate:FindFirstChild("plotDecoration")
                if plotDecoration then
                    for _, child in ipairs(plotDecoration:GetChildren()) do child:Destroy() end
                end
            end
        end
    end)
end

local consoleLines = {"", "", ""}
local consoleLabels = {}

local function updateConsole()
    for i = 1, 3 do
        if consoleLabels[i] then consoleLabels[i].Text = consoleLines[i] end
    end
end

local function logConsole(msg)
    table.remove(consoleLines, 1)
    table.insert(consoleLines, msg)
    updateConsole()
end

local screenGui = Instance.new("ScreenGui")
screenGui.Name = "A"
screenGui.ResetOnSpawn = false
screenGui.Parent = PlayerGui

local mainFrame = Instance.new("Frame")
mainFrame.Size = UDim2.new(0, 200, 0, 120)
mainFrame.Position = UDim2.new(0, 10, 0.5, -60)
mainFrame.BackgroundColor3 = Color3.fromRGB(20, 20, 20)
mainFrame.BorderSizePixel = 0
mainFrame.Active = true
mainFrame.Draggable = true
mainFrame.Parent = screenGui

local title = Instance.new("TextLabel")
title.Size = UDim2.new(1, 0, 0, 20)
title.BackgroundColor3 = Color3.fromRGB(30, 30, 30)
title.BorderSizePixel = 0
title.Text = "A"
title.TextColor3 = Color3.fromRGB(0, 255, 0)
title.TextSize = 12
title.Font = Enum.Font.Code
title.Parent = mainFrame

for i = 1, 3 do
    local line = Instance.new("TextLabel")
    line.Size = UDim2.new(1, -8, 0, 16)
    line.Position = UDim2.new(0, 4, 0, 22 + (i-1) * 18)
    line.BackgroundTransparency = 1
    line.Text = ""
    line.TextColor3 = Color3.fromRGB(200, 200, 200)
    line.TextSize = 11
    line.Font = Enum.Font.Code
    line.TextXAlignment = Enum.TextXAlignment.Left
    line.TextTruncate = Enum.TextTruncate.AtEnd
    line.Parent = mainFrame
    consoleLabels[i] = line
end

local states = {m = false, b = false, c = false}
local stats = {Mythics = 0, Bosses = 0, Cleaned = 0}

local function createToggle(text, key, x)
    local btn = Instance.new("TextButton")
    btn.Size = UDim2.new(0.3, -2, 0, 20)
    btn.Position = UDim2.new(x, 0, 0, 80)
    btn.BackgroundColor3 = Color3.fromRGB(40, 40, 40)
    btn.BorderSizePixel = 0
    btn.Text = text
    btn.TextColor3 = Color3.fromRGB(150, 150, 150)
    btn.TextSize = 10
    btn.Font = Enum.Font.Code
    btn.Parent = mainFrame
    btn.MouseButton1Click:Connect(function()
        states[key] = not states[key]
        btn.BackgroundColor3 = states[key] and Color3.fromRGB(0, 100, 50) or Color3.fromRGB(40, 40, 40)
        btn.TextColor3 = states[key] and Color3.fromRGB(255, 255, 255) or Color3.fromRGB(150, 150, 150)
    end)
end

createToggle("M", "m", 0)
createToggle("B", "b", 0.35)
createToggle("C", "c", 0.7)

local statsLabel = Instance.new("TextLabel")
statsLabel.Size = UDim2.new(1, -8, 0, 14)
statsLabel.Position = UDim2.new(0, 4, 0, 104)
statsLabel.BackgroundTransparency = 1
statsLabel.Text = "M:0 B:0 C:0"
statsLabel.TextColor3 = Color3.fromRGB(100, 200, 255)
statsLabel.TextSize = 10
statsLabel.Font = Enum.Font.Code
statsLabel.TextXAlignment = Enum.TextXAlignment.Left
statsLabel.Parent = mainFrame

local function updateStats()
    statsLabel.Text = string.format("M:%d B:%d C:%d", stats.Mythics, stats.Bosses, stats.Cleaned)
end

local closeBtn = Instance.new("TextButton")
closeBtn.Size = UDim2.new(0, 16, 0, 16)
closeBtn.Position = UDim2.new(1, -18, 0, 2)
closeBtn.BackgroundColor3 = Color3.fromRGB(150, 30, 30)
closeBtn.BorderSizePixel = 0
closeBtn.Text = "X"
closeBtn.TextColor3 = Color3.fromRGB(255, 255, 255)
closeBtn.TextSize = 10
closeBtn.Font = Enum.Font.Code
closeBtn.Parent = mainFrame
closeBtn.MouseButton1Click:Connect(function() screenGui:Destroy() end)

local function getRoot()
    local char = LocalPlayer.Character
    return char and char:FindFirstChild("HumanoidRootPart")
end

local function autoJump()
    if not CONFIG.AUTO_JUMP then return end
    local char = LocalPlayer.Character
    if char then
        local humanoid = char:FindFirstChild("Humanoid")
        if humanoid and humanoid.FloorMaterial ~= Enum.Material.Air then
            humanoid.Jump = true
        end
    end
end

local function moveTo(targetPos, timeout)
    local root = getRoot()
    if not root then return false end
    timeout = timeout or 10
    local startTime = tick()
    while tick() - startTime < timeout do
        root = getRoot()
        if not root then return false end
        if (root.Position - targetPos).Magnitude < 5 then return true end
        root.CFrame = CFrame.new(root.Position, Vector3.new(targetPos.X, root.Position.Y, targetPos.Z))
        root.Velocity = Vector3.new(0, root.Velocity.Y, 0)
        local direction = (targetPos - root.Position).Unit
        root.Velocity = Vector3.new(direction.X * 50, root.Velocity.Y, direction.Z * 50)
        autoJump()
        task.wait(0.05)
    end
    return false
end

local function firePrompt(prompt)
    pcall(function()
        if prompt and prompt.Parent then
            prompt:InputHoldBegin()
            task.wait(0.1)
            prompt:InputHoldEnd()
        end
    end)
end

local shopCategories = {"units", "decoration", "production", "special"}
local buying = false

local function scanShop()
    if buying then return end
    local shopFrame = PlayerGui:FindFirstChild("shopVendor")
    if not shopFrame then return end
    local main = shopFrame:FindFirstChild("main")
    if not main then return end
    local shopFrame2 = main:FindFirstChild("shopFrame")
    if not shopFrame2 then return end

    for _, cat in ipairs(shopCategories) do
        local categoryFrame = shopFrame2:FindFirstChild(cat)
        if categoryFrame then
            for _, item in ipairs(categoryFrame:GetChildren()) do
                local rarity = item:FindFirstChild("rarity")
                if rarity and (rarity:IsA("TextLabel") or rarity:IsA("TextButton")) then
                    if rarity.Text:upper():find("MYTHIC") then
                        local stockFrame = item:FindFirstChild("stockFrame")
                        local stockAmount = stockFrame and stockFrame:FindFirstChild("stockAmount")
                        local stock = stockAmount and tonumber(stockAmount.Text) or 0
                        if stock > 0 then
                            buying = true
                            logConsole('Found mythic "' .. item.Name .. '"')
                            task.wait(0.1)
                            local buyCount = 0
                            while buyCount < stock do
                                PurchaseRemote:FireServer(item.Name)
                                buyCount = buyCount + 1
                                stats.Mythics = stats.Mythics + 1
                                logConsole('Bought ' .. buyCount .. ' "' .. item.Name .. '"')
                                updateStats()
                                task.wait(0.08)
                                local newStock = stockAmount and tonumber(stockAmount.Text) or 0
                                if newStock <= 0 then break end
                            end
                            buying = false
                        end
                    end
                end
            end
        end
    end
end

local capturePoints = {
    {name = "Center", air = nil, ground = nil, naval = nil},
    {name = "1", air = nil, ground = nil, naval = nil},
    {name = "5", air = nil, ground = nil, naval = nil},
}

local function initCapturePoints()
    pcall(function()
        local cp = Workspace.Components.ControlPoints
        if cp then
            if cp:FindFirstChild("Center") then
                local c = cp.Center
                if c:FindFirstChild("interact") then
                    capturePoints[1].air = c.interact:FindFirstChild("AirCaptureTarget")
                    capturePoints[1].ground = c.interact:FindFirstChild("GroundCaptureTarget")
                end
            end
            if cp:FindFirstChild("1") then
                local p = cp["1"]
                if p:FindFirstChild("interact") then
                    capturePoints[2].air = p.interact:FindFirstChild("AirCaptureTarget")
                    capturePoints[2].ground = p.interact:FindFirstChild("GroundCaptureTarget")
                end
            end
            if cp:FindFirstChild("8") then
                local p = cp["8"]
                if p:FindFirstChild("interact") then
                    capturePoints[3].air = p.interact:FindFirstChild("AirCaptureTarget")
                    capturePoints[3].naval = p.interact:FindFirstChild("NavalCaptureTarget")
                end
            end
        end
    end)
end

local huntingBoss = false
local lastBossPos = nil
local currentBossPos = nil

local function getClosestPoint(bossPosition)
    local closest, closestDist = nil, math.huge
    for _, point in ipairs(capturePoints) do
        local target = point.ground or point.air or point.naval
        if target then
            local dist = (target.Position - bossPosition).Magnitude
            if dist < closestDist then
                closestDist = dist
                closest = point
            end
        end
    end
    return closest
end

local function fireAtPoint(cp)
    local root = getRoot()
    if root and cp then
        local oldCFrame = root.CFrame
        local primaryTarget = cp.ground or cp.naval or cp.air
        if primaryTarget then
            root.CFrame = primaryTarget.CFrame
            task.wait(CONFIG.TP_SPEED)
            firePrompt(primaryTarget)
        end
        if cp.air and cp.air ~= primaryTarget then
            root.CFrame = cp.air.CFrame
            task.wait(CONFIG.TP_SPEED)
            firePrompt(cp.air)
        end
        root.CFrame = oldCFrame
    end
end

local function huntBoss()
    if huntingBoss then return end
    local activeUnits = Workspace:FindFirstChild("ActiveUnits")
    if not activeUnits then return end

    local bossFound = false
    
    for _, unit in ipairs(activeUnits:GetChildren()) do
        local bn = unit:GetAttribute("BossName")
        if bn then
            bossFound = true
            huntingBoss = true
            local bossRoot = unit:FindFirstChild("root") or unit:FindFirstChild("HumanoidRootPart") or unit:FindFirstChild("Torso")
            if not bossRoot then
                for _, child in ipairs(unit:GetDescendants()) do
                    if child:IsA("BasePart") then
                        bossRoot = child
                        break
                    end
                end
            end

            if bossRoot then
                local wc = bossRoot:GetAttribute("WorldCFrame")
                local bossPos = wc and Vector3.new(wc.X, wc.Y, wc.Z) or bossRoot.Position
                currentBossPos = bossPos
                local cp = getClosestPoint(bossPos)
                if not lastBossPos then
                    stats.Bosses = stats.Bosses + 1
                    updateStats()
                    logConsole('Found boss "' .. bn .. '"')
                    if cp then
                        local tt = cp.ground and "ground" or (cp.naval and "naval" or "air")
                        logConsole('Closest to "' .. cp.name .. '" (' .. tt .. ')')
                        fireAtPoint(cp)
                    end
                elseif (bossPos - lastBossPos).Magnitude > 10 then
                    if cp then
                        local tt = cp.ground and "ground" or (cp.naval and "naval" or "air")
                        logConsole('Boss moved, redirecting to "' .. cp.name .. '" (' .. tt .. ')')
                        fireAtPoint(cp)
                    end
                end
                lastBossPos = bossPos
            end
            huntingBoss = false
            break
        end
    end
    
    if not bossFound and lastBossPos then
        lastBossPos = nil
        currentBossPos = nil
        logConsole("Boss ended, returning to Center")
        
        local root = getRoot()
        if root and capturePoints[1].ground then
            local oldCFrame = root.CFrame
            root.CFrame = capturePoints[1].ground.CFrame
            task.wait(CONFIG.TP_SPEED)
            firePrompt(capturePoints[1].ground)
            if capturePoints[1].air then
                root.CFrame = capturePoints[1].air.CFrame
                task.wait(CONFIG.TP_SPEED)
                firePrompt(capturePoints[1].air)
            end
            root.CFrame = oldCFrame
        end
    end
end

local function cleanActiveUnits()
    local activeUnits = Workspace:FindFirstChild("ActiveUnits")
    if not activeUnits then return end
    for _, unit in ipairs(activeUnits:GetChildren()) do
        if not unit:GetAttribute("BossName") then
            local maxHealth = unit:GetAttribute("MaxHealth")
            if maxHealth and maxHealth < CONFIG.MAX_HEALTH_DELETE then
                pcall(function() unit:Destroy() end)
                stats.Cleaned = stats.Cleaned + 1
            end
        end
    end
    updateStats()
end

initCapturePoints()
cleanupWorkspace()
logConsole("Ready")

task.spawn(function()
    while screenGui.Parent do
        if states.m then pcall(scanShop) end
        task.wait(CONFIG.REFRESH_RATE)
    end
end)

task.spawn(function()
    while screenGui.Parent do
        if states.b then pcall(huntBoss) end
        task.wait(CONFIG.REFRESH_RATE)
    end
end)

task.spawn(function()
    while screenGui.Parent do
        if states.c then pcall(cleanActiveUnits) end
        task.wait(CONFIG.CLEAN_INTERVAL)
    end
end)

local s = {3, 5, 10, 60}
local i = 1
local g = gethui()
for _, v in pairs(g:GetChildren()) do
    if v.Name == "FPSGui" then v:Destroy() end
end
local sg = Instance.new("ScreenGui", g)
sg.Name = "FPSGui"
sg.ResetOnSpawn = false
local b = Instance.new("TextButton", sg)
b.Size = UDim2.new(0, 80, 0, 25)
b.Position = UDim2.new(1, -90, 0, 10)
b.BackgroundColor3 = Color3.fromRGB(25, 25, 25)
b.BackgroundTransparency = .15
b.Text = "FPS: OFF"
b.TextColor3 = Color3.new(1, 1, 1)
b.TextSize = 12
b.Font = Enum.Font.Code
b.MouseButton1Click:Connect(function()
    i = i % 4 + 1
    local f = s[i]
    if setfpscap then setfpscap(f) end
    b.Text = f ~= 60 and "FPS: " .. f or "FPS: OFF"
end)
