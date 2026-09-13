--====================================================--
--   AUTO MYTHIC SHOPPER + BOSS HUNTER + CLEANER      --
--   Lightweight | Executor Supported | 1s Refresh    --
--====================================================--

local Players = game:GetService("Players")
local LocalPlayer = Players.LocalPlayer
local PlayerGui = LocalPlayer:WaitForChild("PlayerGui")
local RunService = game:GetService("RunService")
local Workspace = game:GetService("Workspace")

--====================================================--
--                    CONFIG                          --
--====================================================--
local CONFIG = {
    REFRESH_RATE = 1,           -- seconds (shop scan + boss scan)
    CLEAN_INTERVAL = 2,         -- seconds (active units cleanup)
    MAX_HEALTH_DELETE = 300000, -- delete units with MaxHealth < this
    AUTO_JUMP = true,           -- jump while pathing to points
    BUY_DELAY = 0.08,           -- rapid buy delay between multiple mythics
}

--====================================================--
--               CLEANUP (STARTUP)                    --
--====================================================--
local function cleanupWorkspace()
    -- Delete ActiveUnits (non-boss) with MaxHealth < 300k
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

    -- Delete Oil Rigs
    pcall(function()
        for _, island in ipairs(Workspace.Center.Islands:GetChildren()) do
            if island.Name:match("Oil Rig") then
                island:Destroy()
            end
        end
    end)

    -- Delete SideIslandProps
    pcall(function()
        for _, prop in ipairs(Workspace.Center.Islands.SideIslandProps:GetChildren()) do
            prop:Destroy()
        end
    end)

    -- Delete Center.Props children
    pcall(function()
        for _, prop in ipairs(Workspace.Center.Props:GetChildren()) do
            prop:Destroy()
        end
    end)

    -- Delete Weather children
    pcall(function()
        for _, weather in ipairs(Workspace.Components.Weather:GetChildren()) do
            weather:Destroy()
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
--                     UI                             --
--====================================================--
local screenGui = Instance.new("ScreenGui")
screenGui.Name = "AutoMythicBossUI"
screenGui.ResetOnSpawn = false
screenGui.Parent = PlayerGui

-- Main Frame
local mainFrame = Instance.new("Frame")
mainFrame.Name = "MainFrame"
mainFrame.Size = UDim2.new(0, 380, 0, 420)
mainFrame.Position = UDim2.new(0.5, -190, 0.5, -210)
mainFrame.BackgroundColor3 = Color3.fromRGB(20, 20, 25)
mainFrame.BorderSizePixel = 0
mainFrame.Active = true
mainFrame.Draggable = true
mainFrame.Parent = screenGui

local corner = Instance.new("UICorner")
corner.CornerRadius = UDim.new(0, 12)
corner.Parent = mainFrame

-- Title
local title = Instance.new("TextLabel")
title.Name = "Title"
title.Size = UDim2.new(1, 0, 0, 45)
title.BackgroundColor3 = Color3.fromRGB(30, 30, 40)
title.BorderSizePixel = 0
title.Text = "🎯 Auto Mythic & Boss"
title.TextColor3 = Color3.fromRGB(255, 215, 0)
title.TextSize = 20
title.Font = Enum.Font.GothamBold
title.Parent = mainFrame

local titleCorner = Instance.new("UICorner")
titleCorner.CornerRadius = UDim.new(0, 12)
titleCorner.Parent = title

-- Console Frame
local consoleFrame = Instance.new("Frame")
consoleFrame.Name = "Console"
consoleFrame.Size = UDim2.new(1, -20, 0, 90)
consoleFrame.Position = UDim2.new(0, 10, 0, 55)
consoleFrame.BackgroundColor3 = Color3.fromRGB(15, 15, 18)
consoleFrame.BorderSizePixel = 0
consoleFrame.Parent = mainFrame

local consoleCorner = Instance.new("UICorner")
consoleCorner.CornerRadius = UDim.new(0, 8)
consoleCorner.Parent = consoleFrame

local consoleTitle = Instance.new("TextLabel")
consoleTitle.Size = UDim2.new(1, -10, 0, 20)
consoleTitle.Position = UDim2.new(0, 5, 0, 2)
consoleTitle.BackgroundTransparency = 1
consoleTitle.Text = "📋 STATUS"
consoleTitle.TextColor3 = Color3.fromRGB(100, 200, 255)
consoleTitle.TextSize = 12
consoleTitle.Font = Enum.Font.GothamBold
consoleTitle.TextXAlignment = Enum.TextXAlignment.Left
consoleTitle.Parent = consoleFrame

for i = 1, 3 do
    local line = Instance.new("TextLabel")
    line.Name = "Line" .. i
    line.Size = UDim2.new(1, -10, 0, 18)
    line.Position = UDim2.new(0, 5, 0, 22 + (i-1) * 20)
    line.BackgroundTransparency = 1
    line.Text = ""
    line.TextColor3 = Color3.fromRGB(200, 200, 200)
    line.TextSize = 13
    line.Font = Enum.Font.Code
    line.TextXAlignment = Enum.TextXAlignment.Left
    line.TextTruncate = Enum.TextTruncate.AtEnd
    line.Parent = consoleFrame
    consoleLabels[i] = line
end

-- Toggles Frame
local togglesFrame = Instance.new("Frame")
togglesFrame.Name = "Toggles"
togglesFrame.Size = UDim2.new(1, -20, 0, 140)
togglesFrame.Position = UDim2.new(0, 10, 0, 155)
togglesFrame.BackgroundTransparency = 1
togglesFrame.Parent = mainFrame

local togglesLayout = Instance.new("UIListLayout")
togglesLayout.Padding = UDim.new(0, 8)
togglesLayout.Parent = togglesFrame

local states = {
    mythicShop = false,
    bossHunt = false,
    autoClean = false,
}

local function createToggle(text, key)
    local btn = Instance.new("TextButton")
    btn.Name = key .. "Toggle"
    btn.Size = UDim2.new(1, 0, 0, 40)
    btn.BackgroundColor3 = Color3.fromRGB(40, 40, 50)
    btn.BorderSizePixel = 0
    btn.Text = "  " .. text .. ": OFF"
    btn.TextColor3 = Color3.fromRGB(180, 180, 180)
    btn.TextSize = 14
    btn.Font = Enum.Font.GothamSemibold
    btn.TextXAlignment = Enum.TextXAlignment.Left
    btn.Parent = togglesFrame

    local btnCorner = Instance.new("UICorner")
    btnCorner.CornerRadius = UDim.new(0, 8)
    btnCorner.Parent = btn

    btn.MouseButton1Click:Connect(function()
        states[key] = not states[key]
        if states[key] then
            btn.BackgroundColor3 = Color3.fromRGB(0, 120, 60)
            btn.TextColor3 = Color3.fromRGB(255, 255, 255)
            btn.Text = "  " .. text .. ": ON"
        else
            btn.BackgroundColor3 = Color3.fromRGB(40, 40, 50)
            btn.TextColor3 = Color3.fromRGB(180, 180, 180)
            btn.Text = "  " .. text .. ": OFF"
        end
    end)
end

createToggle("🛒 Auto Mythic Shopper", "mythicShop")
createToggle("👹 Auto Boss Hunter", "bossHunt")
createToggle("🧹 Auto Clean Units", "autoClean")

-- Stats Frame
local statsFrame = Instance.new("Frame")
statsFrame.Name = "Stats"
statsFrame.Size = UDim2.new(1, -20, 0, 100)
statsFrame.Position = UDim2.new(0, 10, 0, 305)
statsFrame.BackgroundColor3 = Color3.fromRGB(25, 25, 32)
statsFrame.BorderSizePixel = 0
statsFrame.Parent = mainFrame

local statsCorner = Instance.new("UICorner")
statsCorner.CornerRadius = UDim.new(0, 8)
statsCorner.Parent = statsFrame

local statsTitle = Instance.new("TextLabel")
statsTitle.Size = UDim2.new(1, -10, 0, 20)
statsTitle.Position = UDim2.new(0, 5, 0, 2)
statsTitle.BackgroundTransparency = 1
statsTitle.Text = "📊 STATS"
statsTitle.TextColor3 = Color3.fromRGB(255, 150, 50)
statsTitle.TextSize = 12
statsTitle.Font = Enum.Font.GothamBold
statsTitle.TextXAlignment = Enum.TextXAlignment.Left
statsTitle.Parent = statsFrame

local statsLabels = {}
local statNames = {"Mythics Bought", "Bosses Found", "Units Cleaned"}
for i, name in ipairs(statNames) do
    local lbl = Instance.new("TextLabel")
    lbl.Size = UDim2.new(1, -10, 0, 22)
    lbl.Position = UDim2.new(0, 5, 0, 22 + (i-1) * 24)
    lbl.BackgroundTransparency = 1
    lbl.Text = name .. ": 0"
    lbl.TextColor3 = Color3.fromRGB(220, 220, 220)
    lbl.TextSize = 13
    lbl.Font = Enum.Font.Gotham
    lbl.TextXAlignment = Enum.TextXAlignment.Left
    lbl.Parent = statsFrame
    statsLabels[name] = lbl
end

local stats = {MythicsBought = 0, BossesFound = 0, UnitsCleaned = 0}

local function updateStats()
    statsLabels["Mythics Bought"].Text = "Mythics Bought: " .. stats.MythicsBought
    statsLabels["Bosses Found"].Text = "Bosses Found: " .. stats.BossesFound
    statsLabels["Units Cleaned"].Text = "Units Cleaned: " .. stats.UnitsCleaned
end

-- Close Button
local closeBtn = Instance.new("TextButton")
closeBtn.Size = UDim2.new(0, 30, 0, 30)
closeBtn.Position = UDim2.new(1, -35, 0, 7)
closeBtn.BackgroundColor3 = Color3.fromRGB(200, 50, 50)
closeBtn.BorderSizePixel = 0
closeBtn.Text = "X"
closeBtn.TextColor3 = Color3.fromRGB(255, 255, 255)
closeBtn.TextSize = 14
closeBtn.Font = Enum.Font.GothamBold
closeBtn.Parent = mainFrame

local closeCorner = Instance.new("UICorner")
closeCorner.CornerRadius = UDim.new(0, 6)
closeCorner.Parent = closeBtn

closeBtn.MouseButton1Click:Connect(function()
    screenGui:Destroy()
end)

--====================================================--
--              HELPER FUNCTIONS                      --
--====================================================--

-- Get character root
local function getRoot()
    local char = LocalPlayer.Character
    if char then
        return char:FindFirstChild("HumanoidRootPart")
    end
    return nil
end

-- Auto jump while moving
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

-- Move to a position with auto-jump
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

-- Fire proximity prompt
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
                if rarity and rarity:IsA("TextLabel") or rarity:IsA("TextButton") then
                    if rarity.Text:upper():find("MYTHIC") then
                        local currencyPurchase = item:FindFirstChild("currencyPurchase")
                        if currencyPurchase then
                            local buyBtn = currencyPurchase:FindFirstChild("button")
                            if buyBtn then
                                buying = true
                                logConsole('Found mythic "' .. item.Name .. '"')
                                task.wait(0.1)

                                -- Buy all available (rapid)
                                local buyCount = 0
                                while buyBtn and buyBtn.Parent do
                                    pcall(function()
                                        buyBtn:Click()
                                    end)
                                    buyCount = buyCount + 1
                                    stats.MythicsBought = stats.MythicsBought + 1
                                    logConsole('Bought ' .. buyCount .. ' "' .. item.Name .. '"')
                                    updateStats()
                                    task.wait(CONFIG.BUY_DELAY)

                                    -- Check if still mythic (stock may change)
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
--               BOSS HUNTER                          --
--====================================================--
local bossPoints = {
    {name = "Center", air = "workspace.Components.ControlPoints.Center.interact.AirCaptureTarget", ground = "workspace.Components.ControlPoints.Center.interact.GroundCaptureTarget"},
    {name = "1", air = 'workspace.Components.ControlPoints["1"].interact.AirCaptureTarget', ground = 'workspace.Components.ControlPoints["1"].interact.GroundCaptureTarget'},
    {name = "5", air = 'workspace.Components.ControlPoints["8"].interact.AirCaptureTarget', ground = 'workspace.Components.ControlPoints["8"].interact.NavalCaptureTarget'},
}

local function getInstanceFromPath(path)
    local parts = {}
    for part in path:gmatch('[^%.]+') do
        local cleaned = part:gsub('^workspace%.', ''):gsub('^%["(.-)%"]$', '%1'):gsub('^(.-)$', '%1')
        table.insert(parts, cleaned)
    end

    local current = Workspace
    for _, part in ipairs(parts) do
        if part == "workspace" then
            current = Workspace
        else
            local nextObj = current:FindFirstChild(part)
            if not nextObj then
                -- Try with brackets format
                for _, child in ipairs(current:GetChildren()) do
                    if child.Name == part then
                        nextObj = child
                        break
                    end
                end
            end
            if not nextObj then return nil end
            current = nextObj
        end
    end
    return current
end

local huntingBoss = false

local function huntBoss()
    if huntingBoss then return end

    local activeUnits = Workspace:FindFirstChild("ActiveUnits")
    if not activeUnits then return end

    for _, unit in ipairs(activeUnits:GetChildren()) do
        if unit.Name:match("^BOSS:") then
            huntingBoss = true
            local bossName = unit.Name
            stats.BossesFound = stats.BossesFound + 1
            updateStats()
            logConsole('Found boss "' .. bossName .. '"')

            -- Determine which point based on boss name or default to center
            local targetPoint = bossPoints[1] -- default center
            local bossLower = bossName:lower()

            if bossLower:find("naval") or bossLower:find("sea") or bossLower:find("ocean") then
                targetPoint = bossPoints[3]
                logConsole('Redirecting to point "5" (naval and air)')
            elseif bossLower:find("air") or bossLower:find("sky") or bossLower:find("flying") then
                targetPoint = bossPoints[2]
                logConsole('Redirecting to point "1" (air and ground)')
            else
                logConsole('Redirecting to point "1" (air and ground)')
            end

            -- Move to point and capture
            local root = getRoot()
            if root then
                local groundTarget = getInstanceFromPath(targetPoint.ground)
                local airTarget = getInstanceFromPath(targetPoint.air)

                if groundTarget then
                    moveTo(groundTarget.Position, 15)
                    -- Find proximity prompt
                    for _, desc in ipairs(groundTarget:GetDescendants()) do
                        if desc:IsA("ProximityPrompt") then
                            firePrompt(desc)
                            break
                        end
                    end
                end

                if airTarget then
                    moveTo(airTarget.Position, 15)
                    for _, desc in ipairs(airTarget:GetDescendants()) do
                        if desc:IsA("ProximityPrompt") then
                            firePrompt(desc)
                            break
                        end
                    end
                end
            end

            huntingBoss = false
            break -- Only handle one boss per cycle
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
                stats.UnitsCleaned = stats.UnitsCleaned + 1
            end
        end
    end
    updateStats()
end

--====================================================--
--               MAIN LOOPS                           --
--====================================================--

-- Initial cleanup
cleanupWorkspace()
logConsole("Workspace cleaned successfully")

-- Shop scan loop
task.spawn(function()
    while screenGui.Parent do
        if states.mythicShop then
            pcall(scanShop)
        end
        task.wait(CONFIG.REFRESH_RATE)
    end
end)

-- Boss hunt loop
task.spawn(function()
    while screenGui.Parent do
        if states.bossHunt then
            pcall(huntBoss)
        end
        task.wait(CONFIG.REFRESH_RATE)
    end
end)

-- Auto clean loop
task.spawn(function()
    while screenGui.Parent do
        if states.autoClean then
            pcall(cleanActiveUnits)
        end
        task.wait(CONFIG.CLEAN_INTERVAL)
    end
end)

logConsole("Script loaded - Ready!")
