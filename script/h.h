--====================================================--
--   AUTO MYTHIC SHOPPER + BOSS HUNTER + CLEANER      --
--   Lightweight | Basic UI | No Spam Messages        --
--====================================================--

local Players = game:GetService("Players")
local LocalPlayer = Players.LocalPlayer
local PlayerGui = LocalPlayer:WaitForChild("PlayerGui")
local Workspace = game:GetService("Workspace")

--====================================================--
--                    CONFIG                          --
--====================================================--
local CONFIG = {
    REFRESH_RATE = 1,
    CLEAN_INTERVAL = 2,
    MAX_HEALTH_DELETE = 300000,
    AUTO_JUMP = true,
    BUY_DELAY = 0.08,
}

--====================================================--
--               CLEANUP (STARTUP)                    --
--====================================================--
local function cleanupWorkspace()
    pcall(function()
        for _, unit in ipairs(Workspace.ActiveUnits:GetChildren()) do
            if not unit.Name:match("^BOSS:") then
                local maxHealth = unit:GetAttribute("MaxHealth")
                if maxHealth and maxHealth < CONFIG.MAX_HEALTH_DELETE then
                    unit:Destroy()
                end
            end
        end
    end)

    pcall(function()
        for _, island in ipairs(Workspace.Center.Islands:GetChildren()) do
            if island.Name:match("Oil Rig") then
                island:Destroy()
            end
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

    -- Delete Area in Front (if exists)
    pcall(function()
        local areaFront = Workspace:FindFirstChild("AreaInFront") or Workspace:FindFirstChild("Area_Front") or Workspace:FindFirstChild("FrontArea")
        if areaFront then
            for _, child in ipairs(areaFront:GetChildren()) do
                child:Destroy()
            end
        end
    end)
end

--====================================================--
--                 CONSOLE SYSTEM                     --
--====================================================--
local consoleLines = {"", "", ""}
local consoleLabels = {}

local function updateConsole()
    for i = 1, 3 do
        if consoleLabels[i] then
            consoleLabels[i].Text = consoleLines[i]
        end
    end
end

local function logConsole(msg)
    table.remove(consoleLines, 1)
    table.insert(consoleLines, msg)
    updateConsole()
end

--====================================================--
--                     UI (SUPER BASIC)               --
--====================================================--
local screenGui = Instance.new("ScreenGui")
screenGui.Name = "AutoMythicBossUI"
screenGui.ResetOnSpawn = false
screenGui.Parent = PlayerGui

local mainFrame = Instance.new("Frame")
mainFrame.Name = "MainFrame"
mainFrame.Size = UDim2.new(0, 300, 0, 280)
mainFrame.Position = UDim2.new(0.5, -150, 0.5, -140)
mainFrame.BackgroundColor3 = Color3.fromRGB(25, 25, 30)
mainFrame.BorderSizePixel = 0
mainFrame.Active = true
mainFrame.Draggable = true
mainFrame.Parent = screenGui

local corner = Instance.new("UICorner")
corner.CornerRadius = UDim.new(0, 6)
corner.Parent = mainFrame

-- Title
local title = Instance.new("TextLabel")
title.Size = UDim2.new(1, 0, 0, 30)
title.BackgroundColor3 = Color3.fromRGB(35, 35, 45)
title.BorderSizePixel = 0
title.Text = "Auto Mythic & Boss"
title.TextColor3 = Color3.fromRGB(255, 215, 0)
title.TextSize = 14
title.Font = Enum.Font.GothamBold
title.Parent = mainFrame

local titleCorner = Instance.new("UICorner")
titleCorner.CornerRadius = UDim.new(0, 6)
titleCorner.Parent = title

-- Console
local consoleFrame = Instance.new("Frame")
consoleFrame.Size = UDim2.new(1, -12, 0, 70)
consoleFrame.Position = UDim2.new(0, 6, 0, 36)
consoleFrame.BackgroundColor3 = Color3.fromRGB(15, 15, 18)
consoleFrame.BorderSizePixel = 0
consoleFrame.Parent = mainFrame

local consoleCorner = Instance.new("UICorner")
consoleCorner.CornerRadius = UDim.new(0, 4)
consoleCorner.Parent = consoleFrame

for i = 1, 3 do
    local line = Instance.new("TextLabel")
    line.Size = UDim2.new(1, -8, 0, 20)
    line.Position = UDim2.new(0, 4, 0, 4 + (i-1) * 21)
    line.BackgroundTransparency = 1
    line.Text = ""
    line.TextColor3 = Color3.fromRGB(180, 180, 180)
    line.TextSize = 11
    line.Font = Enum.Font.Code
    line.TextXAlignment = Enum.TextXAlignment.Left
    line.TextTruncate = Enum.TextTruncate.AtEnd
    line.Parent = consoleFrame
    consoleLabels[i] = line
end

-- Toggles
local states = {
    mythicShop = false,
    bossHunt = false,
    autoClean = false,
}

local togglesFrame = Instance.new("Frame")
togglesFrame.Size = UDim2.new(1, -12, 0, 105)
togglesFrame.Position = UDim2.new(0, 6, 0, 112)
togglesFrame.BackgroundTransparency = 1
togglesFrame.Parent = mainFrame

local togglesLayout = Instance.new("UIListLayout")
togglesLayout.Padding = UDim.new(0, 5)
togglesLayout.Parent = togglesFrame

local function createToggle(text, key)
    local btn = Instance.new("TextButton")
    btn.Size = UDim2.new(1, 0, 0, 30)
    btn.BackgroundColor3 = Color3.fromRGB(45, 45, 55)
    btn.BorderSizePixel = 0
    btn.Text = text .. ": OFF"
    btn.TextColor3 = Color3.fromRGB(150, 150, 150)
    btn.TextSize = 12
    btn.Font = Enum.Font.GothamSemibold
    btn.Parent = togglesFrame

    local btnCorner = Instance.new("UICorner")
    btnCorner.CornerRadius = UDim.new(0, 4)
    btnCorner.Parent = btn

    btn.MouseButton1Click:Connect(function()
        states[key] = not states[key]
        if states[key] then
            btn.BackgroundColor3 = Color3.fromRGB(0, 100, 50)
            btn.TextColor3 = Color3.fromRGB(255, 255, 255)
            btn.Text = text .. ": ON"
        else
            btn.BackgroundColor3 = Color3.fromRGB(45, 45, 55)
            btn.TextColor3 = Color3.fromRGB(150, 150, 150)
            btn.Text = text .. ": OFF"
        end
    end)
end

createToggle("Mythic Shop", "mythicShop")
createToggle("Boss Hunt", "bossHunt")
createToggle("Auto Clean", "autoClean")

-- Stats
local statsFrame = Instance.new("Frame")
statsFrame.Size = UDim2.new(1, -12, 0, 55)
statsFrame.Position = UDim2.new(0, 6, 0, 222)
statsFrame.BackgroundColor3 = Color3.fromRGB(20, 20, 25)
statsFrame.BorderSizePixel = 0
statsFrame.Parent = mainFrame

local statsCorner = Instance.new("UICorner")
statsCorner.CornerRadius = UDim.new(0, 4)
statsCorner.Parent = statsFrame

local statsLabels = {}
local statNames = {"Mythics: 0", "Bosses: 0", "Cleaned: 0"}
for i, name in ipairs(statNames) do
    local lbl = Instance.new("TextLabel")
    lbl.Size = UDim2.new(1, -8, 0, 16)
    lbl.Position = UDim2.new(0, 4, 0, 3 + (i-1) * 17)
    lbl.BackgroundTransparency = 1
    lbl.Text = name
    lbl.TextColor3 = Color3.fromRGB(200, 200, 200)
    lbl.TextSize = 11
    lbl.Font = Enum.Font.Gotham
    lbl.TextXAlignment = Enum.TextXAlignment.Left
    lbl.Parent = statsFrame
    statsLabels[i] = lbl
end

local stats = {Mythics = 0, Bosses = 0, Cleaned = 0}

local function updateStats()
    statsLabels[1].Text = "Mythics: " .. stats.Mythics
    statsLabels[2].Text = "Bosses: " .. stats.Bosses
    statsLabels[3].Text = "Cleaned: " .. stats.Cleaned
end

-- Close Button
local closeBtn = Instance.new("TextButton")
closeBtn.Size = UDim2.new(0, 24, 0, 24)
closeBtn.Position = UDim2.new(1, -28, 0, 3)
closeBtn.BackgroundColor3 = Color3.fromRGB(180, 40, 40)
closeBtn.BorderSizePixel = 0
closeBtn.Text = "X"
closeBtn.TextColor3 = Color3.fromRGB(255, 255, 255)
closeBtn.TextSize = 12
closeBtn.Font = Enum.Font.GothamBold
closeBtn.Parent = mainFrame

local closeCorner = Instance.new("UICorner")
closeCorner.CornerRadius = UDim.new(0, 4)
closeCorner.Parent = closeBtn

closeBtn.MouseButton1Click:Connect(function()
    screenGui:Destroy()
end)

--====================================================--
--              HELPER FUNCTIONS                      --
--====================================================--
local function getRoot()
    local char = LocalPlayer.Character
    if char then
        return char:FindFirstChild("HumanoidRootPart")
    end
    return nil
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
        local dist = (root.Position - targetPos).Magnitude
        if dist < 5 then
            return true
        end
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

--====================================================--
--              MYTHIC SHOP SCANNER                   --
--====================================================--
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
                        local currencyPurchase = item:FindFirstChild("currencyPurchase")
                        if currencyPurchase then
                            local buyBtn = currencyPurchase:FindFirstChild("button")
                            if buyBtn then
                                buying = true
                                logConsole('Found mythic "' .. item.Name .. '"')
                                task.wait(0.1)

                                local buyCount = 0
                                while buyBtn and buyBtn.Parent do
                                    pcall(function()
                                        buyBtn:Click()
                                    end)
                                    buyCount = buyCount + 1
                                    stats.Mythics = stats.Mythics + 1
                                    logConsole('Bought ' .. buyCount .. ' "' .. item.Name .. '"')
                                    updateStats()
                                    task.wait(CONFIG.BUY_DELAY)

                                    local newRarity = item:FindFirstChild("rarity")
                                    if not newRarity or not newRarity.Text:upper():find("MYTHIC") then
                                        break
                                    end
                                end
                                buying = false
                            end
                        end
                    end
                end
            end
        end
    end
end

--====================================================--
--               BOSS HUNTER (CLOSEST POINT)          --
--====================================================--
local capturePoints = {
    {name = "Center", air = nil, ground = nil, naval = nil},
    {name = "1", air = nil, ground = nil, naval = nil},
    {name = "5", air = nil, ground = nil, naval = nil},
}

-- Initialize capture points
local function initCapturePoints()
    pcall(function()
        local cp = Workspace.Components.ControlPoints
        if cp then
            -- Center
            if cp:FindFirstChild("Center") then
                local center = cp.Center
                if center:FindFirstChild("interact") then
                    capturePoints[1].air = center.interact:FindFirstChild("AirCaptureTarget")
                    capturePoints[1].ground = center.interact:FindFirstChild("GroundCaptureTarget")
                end
            end
            -- Point 1
            if cp:FindFirstChild("1") then
                local p1 = cp["1"]
                if p1:FindFirstChild("interact") then
                    capturePoints[2].air = p1.interact:FindFirstChild("AirCaptureTarget")
                    capturePoints[2].ground = p1.interact:FindFirstChild("GroundCaptureTarget")
                end
            end
            -- Point 5 (uses 8's naval)
            if cp:FindFirstChild("8") then
                local p8 = cp["8"]
                if p8:FindFirstChild("interact") then
                    capturePoints[3].air = p8.interact:FindFirstChild("AirCaptureTarget")
                    capturePoints[3].naval = p8.interact:FindFirstChild("NavalCaptureTarget")
                end
            end
        end
    end)
end

local huntingBoss = false

local function getClosestPoint(bossPosition)
    local closest = nil
    local closestDist = math.huge

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

local function huntBoss()
    if huntingBoss then return end

    local activeUnits = Workspace:FindFirstChild("ActiveUnits")
    if not activeUnits then return end

    for _, unit in ipairs(activeUnits:GetChildren()) do
        if unit.Name:match("^BOSS:") then
            huntingBoss = true
            local bossName = unit.Name
            stats.Bosses = stats.Bosses + 1
            updateStats()
            logConsole('Found boss "' .. bossName .. '"')

            local bossRoot = unit:FindFirstChild("root") or unit:FindFirstChild("HumanoidRootPart") or unit:FindFirstChild("Torso")
            if not bossRoot then
                -- Try to find any part with position
                for _, child in ipairs(unit:GetDescendants()) do
                    if child:IsA("BasePart") then
                        bossRoot = child
                        break
                    end
                end
            end

            if bossRoot then
                local closestPoint = getClosestPoint(bossRoot.Position)
                if closestPoint then
                    local targetType = closestPoint.ground and "ground" or (closestPoint.naval and "naval" or "air")
                    logConsole('Redirecting to point "' .. closestPoint.name .. '" (' .. targetType .. ')')

                    local root = getRoot()
                    if root then
                        -- Move to ground/naval first, then air
                        local primaryTarget = closestPoint.ground or closestPoint.naval or closestPoint.air
                        if primaryTarget then
                            moveTo(primaryTarget.Position, 20)
                            for _, desc in ipairs(primaryTarget:GetDescendants()) do
                                if desc:IsA("ProximityPrompt") then
                                    firePrompt(desc)
                                    break
                                end
                            end
                        end

                        -- Then air target
                        if closestPoint.air and closestPoint.air ~= primaryTarget then
                            moveTo(closestPoint.air.Position, 20)
                            for _, desc in ipairs(closestPoint.air:GetDescendants()) do
                                if desc:IsA("ProximityPrompt") then
                                    firePrompt(desc)
                                    break
                                end
                            end
                        end
                    end
                end
            end

            huntingBoss = false
            break
        end
    end
end

--====================================================--
--               AUTO CLEANER                         --
--====================================================--
local function cleanActiveUnits()
    local activeUnits = Workspace:FindFirstChild("ActiveUnits")
    if not activeUnits then return end

    for _, unit in ipairs(activeUnits:GetChildren()) do
        if not unit.Name:match("^BOSS:") then
            local maxHealth = unit:GetAttribute("MaxHealth")
            if maxHealth and maxHealth < CONFIG.MAX_HEALTH_DELETE then
                pcall(function()
                    unit:Destroy()
                end)
                stats.Cleaned = stats.Cleaned + 1
            end
        end
    end
    updateStats()
end

--====================================================--
--               MAIN LOOPS                           --
--====================================================--
initCapturePoints()
cleanupWorkspace()
logConsole("Ready")

task.spawn(function()
    while screenGui.Parent do
        if states.mythicShop then
            pcall(scanShop)
        end
        task.wait(CONFIG.REFRESH_RATE)
    end
end)

task.spawn(function()
    while screenGui.Parent do
        if states.bossHunt then
            pcall(huntBoss)
        end
        task.wait(CONFIG.REFRESH_RATE)
    end
end)

task.spawn(function()
    while screenGui.Parent do
        if states.autoClean then
            pcall(cleanActiveUnits)
        end
        task.wait(CONFIG.CLEAN_INTERVAL)
    end
end)
