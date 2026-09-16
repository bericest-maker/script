local Players=game:GetService("Players")
local LocalPlayer=Players.LocalPlayer
local PlayerGui=LocalPlayer:WaitForChild("PlayerGui")
local Workspace=game:GetService("Workspace")
local RS=game:GetService("ReplicatedStorage")

local CONFIG={REFRESH_RATE=1,CLEAN_INTERVAL=2,MAX_HEALTH_DELETE=300000,TP_SPEED=0.3}
local GRAY=Color3.fromRGB(212,208,200) local DARK=Color3.fromRGB(80,80,80)

local PurchaseRemote=RS:WaitForChild("Shared"):WaitForChild("Resources"):WaitForChild("VendorResources"):WaitForChild("Remotes"):WaitForChild("PurchaseStructure")

local function cleanupWorkspace()
	pcall(function() local t=Workspace:FindFirstChild("Terrain") if t then t:Destroy() end end)
	pcall(function() for _,c in ipairs(Workspace.Center:GetChildren()) do if c.Name~="OceanCollision" then c:Destroy() end end end)
	pcall(function() for _,p in ipairs(Workspace.Plots:GetChildren()) do p:Destroy() end end)
	pcall(function() for _,w in ipairs(Workspace.Components.Weather:GetChildren()) do w:Destroy() end end)
end

local lines={"","",""} local labels={}
local function log(m) table.remove(lines,1) table.insert(lines,m) for i=1,3 do if labels[i] then labels[i].Text=lines[i] end end end

local sg=Instance.new("ScreenGui") sg.Name="A" sg.ResetOnSpawn=false sg.Parent=PlayerGui

local W,H=150,92
local f=Instance.new("Frame")
f.Size=UDim2.new(0,W,0,H) f.Position=UDim2.new(0,10,0.5,-H/2)
f.BackgroundColor3=GRAY f.BorderSizePixel=0 f.Active=true f.Draggable=true f.Parent=sg

local tb=Instance.new("Frame") tb.Size=UDim2.new(1,0,0,16) tb.BackgroundColor3=GRAY tb.BorderSizePixel=0 tb.Parent=f
local tl=Instance.new("TextLabel") tl.Size=UDim2.new(1,-40,1,0) tl.Position=UDim2.new(0,20,0,0) tl.BackgroundTransparency=1 tl.Text="Name" tl.TextColor3=Color3.new(0,0,0) tl.TextSize=10 tl.Font=Enum.Font.Code tl.Parent=tb

local function mkBtn(txt,x)
	local b=Instance.new("TextButton")
	b.Size=UDim2.new(0,15,0,12) b.Position=UDim2.new(1,x,0,2)
	b.BackgroundColor3=GRAY b.BorderSizePixel=1 b.BorderColor3=DARK
	b.Text=txt b.TextColor3=Color3.new(0,0,0) b.TextSize=10 b.Font=Enum.Font.Code b.Parent=tb
	return b
end

local minB=mkBtn("_",-32) local xB=mkBtn("X",-16)

local body=Instance.new("Frame") body.Size=UDim2.new(1,0,1,-16) body.Position=UDim2.new(0,0,0,16) body.BackgroundTransparency=1 body.BorderSizePixel=0 body.Parent=f

local cb=Instance.new("Frame") cb.Size=UDim2.new(1,-6,0,36) cb.Position=UDim2.new(0,3,0,3) cb.BackgroundColor3=Color3.fromRGB(235,235,235) cb.BorderSizePixel=1 cb.BorderColor3=DARK cb.Parent=body
for i=1,3 do
	local l=Instance.new("TextLabel")
	l.Size=UDim2.new(1,-4,0,11) l.Position=UDim2.new(0,2,0,1+(i-1)*11)
	l.BackgroundTransparency=1 l.Text="" l.TextColor3=Color3.new(0,0,0) l.TextSize=10 l.Font=Enum.Font.Code
	l.TextXAlignment=Enum.TextXAlignment.Left l.TextTruncate=Enum.TextTruncate.AtEnd l.Parent=cb
	labels[i]=l
end

local states={m=false,b=false,c=false}
local stats={M=0,B=0,C=0}

local sLabel=Instance.new("TextLabel")
sLabel.Size=UDim2.new(1,-6,0,11) sLabel.Position=UDim2.new(0,3,0,75)
sLabel.BackgroundTransparency=1 sLabel.Text="M:0 B:0 C:0" sLabel.TextColor3=DARK sLabel.TextSize=10 sLabel.Font=Enum.Font.Code
sLabel.TextXAlignment=Enum.TextXAlignment.Left sLabel.Parent=body

local function updStats() sLabel.Text=string.format("M:%d B:%d C:%d",stats.M,stats.B,stats.C) end

for i,k in ipairs({"m","b","c"}) do
	local b=Instance.new("TextButton")
	b.Size=UDim2.new(0.333,-3,0,14) b.Position=UDim2.new((i-1)*0.333,2,0,42)
	b.BackgroundColor3=GRAY b.BorderSizePixel=1 b.BorderColor3=DARK
	b.Text=k:upper() b.TextColor3=Color3.new(0,0,0) b.TextSize=10 b.Font=Enum.Font.Code b.Parent=body
	b.MouseButton1Click:Connect(function()
		states[k]=not states[k]
		b.BackgroundColor3=states[k] and Color3.new(1,1,1) or GRAY
		b.BorderColor3=states[k] and Color3.new(0,0,0) or DARK
	end)
end

local minimized=false
minB.MouseButton1Click:Connect(function()
	minimized=not minimized body.Visible=not minimized
	f.Size=minimized and UDim2.new(0,W,0,16) or UDim2.new(0,W,0,H)
end)
xB.MouseButton1Click:Connect(function() sg:Destroy() end)

local function getRoot() local c=LocalPlayer.Character return c and c:FindFirstChild("HumanoidRootPart") end

local function firePrompt(p)
	pcall(function() if p and p.Parent then p:InputHoldBegin() task.wait(0.1) p:InputHoldEnd() end end)
end

local buying=false
local function scanShop()
	if buying then return end
	local sf=PlayerGui:FindFirstChild("shopVendor") if not sf then return end
	local m=sf:FindFirstChild("main") if not m then return end
	local sf2=m:FindFirstChild("shopFrame") if not sf2 then return end
	for _,cat in ipairs({"units","decoration","production","special"}) do
		local cf=sf2:FindFirstChild(cat)
		if cf then
			for _,item in ipairs(cf:GetChildren()) do
				local r=item:FindFirstChild("rarity")
				if r and r.Text:upper():find("MYTHIC") then
					local sa=item:FindFirstChild("stockFrame") and item.stockFrame:FindFirstChild("stockAmount")
					local stock=sa and tonumber(sa.Text) or 0
					if stock>0 then
						buying=true
						log('Found mythic "'..item.Name..'"')
						task.wait(0.1)
						local bc=0
						while bc<stock do
							PurchaseRemote:FireServer(item.Name)
							bc=bc+1 stats.M=stats.M+1
							log('Bought '..bc..' "'..item.Name..'"') updStats()
							task.wait(0.08)
							if (sa and tonumber(sa.Text) or 0)<=0 then break end
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

local function initCP()
	pcall(function()
		local cp=Workspace.Components.ControlPoints
		local function grab(p,t) local i=p:FindFirstChild("interact") if i then t.air=i:FindFirstChild("AirCaptureTarget") t.ground=i:FindFirstChild("GroundCaptureTarget") t.naval=i:FindFirstChild("NavalCaptureTarget") end end
		if cp:FindFirstChild("Center") then grab(cp.Center,capturePoints[1]) end
		if cp:FindFirstChild("1") then grab(cp["1"],capturePoints[2]) end
		if cp:FindFirstChild("8") then grab(cp["8"],capturePoints[3]) end
	end)
end

local hunting=false local lastBoss=nil local currentBossPos=nil

local function closestPoint(bp)
	local cl,cd=nil,math.huge
	for _,p in ipairs(capturePoints) do
		local t=p.ground or p.air or p.naval
		if t then local d=(t.Position-bp).Magnitude if d<cd then cd=d cl=p end end
	end
	return cl
end

local function tpTo(cp)
	local r=getRoot() if not r then return end
	local oc=r.CFrame
	local pt=cp.ground or cp.naval or cp.air
	if pt then r.CFrame=pt.CFrame task.wait(CONFIG.TP_SPEED) firePrompt(pt) end
	if cp.air and cp.air~=pt then r.CFrame=cp.air.CFrame task.wait(CONFIG.TP_SPEED) firePrompt(cp.air) end
	r.CFrame=oc
end

local function huntBoss()
	if hunting then return end
	local au=Workspace:FindFirstChild("ActiveUnits") if not au then return end
	local found=false
	for _,u in ipairs(au:GetChildren()) do
		local bn=u:GetAttribute("BossName")
		if bn then
			found=true hunting=true
			local br=u:FindFirstChild("root") or u:FindFirstChild("HumanoidRootPart") or u:FindFirstChild("Torso")
			if not br then for _,c in ipairs(u:GetDescendants()) do if c:IsA("BasePart") then br=c break end end end
			if br then
				local wc=br:GetAttribute("WorldCFrame")
				local bp=wc and Vector3.new(wc.X,wc.Y,wc.Z) or br.Position
				currentBossPos=bp
				local cp=closestPoint(bp)
				if not lastBoss then
					stats.B=stats.B+1 updStats()
					log('Found boss "'..bn..'"')
					if cp then
						local tt=cp.ground and "ground" or (cp.naval and "naval" or "air")
						log('Closest to "'..cp.name..'" ('..tt..')')
						tpTo(cp)
					end
				elseif (bp-lastBoss).Magnitude>10 then
					if cp then
						local tt=cp.ground and "ground" or (cp.naval and "naval" or "air")
						log('Boss moved -> "'..cp.name..'" ('..tt..')')
						tpTo(cp)
					end
				end
				lastBoss=bp
			end
			hunting=false
			break
		end
	end
	if not found and lastBoss then
		lastBoss=nil currentBossPos=nil
		log("Boss ended, returning to Center")
		if capturePoints[1].ground then tpTo(capturePoints[1]) end
	end
end

local function cleanUnits()
	local au=Workspace:FindFirstChild("ActiveUnits") if not au then return end
	for _,u in ipairs(au:GetChildren()) do
		if not u:GetAttribute("BossName") then
			local mh=u:GetAttribute("MaxHealth")
			if mh and mh<CONFIG.MAX_HEALTH_DELETE then
				pcall(function() u:Destroy() end)
				stats.C=stats.C+1
			end
		end
	end
	updStats()
end

initCP() cleanupWorkspace() log("Ready")

task.spawn(function() while sg.Parent do if states.m then pcall(scanShop) end task.wait(CONFIG.REFRESH_RATE) end end)
task.spawn(function() while sg.Parent do if states.b then pcall(huntBoss) end task.wait(CONFIG.REFRESH_RATE) end end)
task.spawn(function() while sg.Parent do if states.c then pcall(cleanUnits) end task.wait(CONFIG.CLEAN_INTERVAL) end end)

local fps={3,5,10,60} local fi=1
local g=gethui()
for _,v in pairs(g:GetChildren()) do if v.Name=="FPSGui" then v:Destroy() end end
local fg=Instance.new("ScreenGui",g) fg.Name="FPSGui" fg.ResetOnSpawn=false
local fb=Instance.new("TextButton",fg)
fb.Size=UDim2.new(0,70,0,16) fb.Position=UDim2.new(1,-80,0,10)
fb.BackgroundColor3=GRAY fb.BorderSizePixel=1 fb.BorderColor3=DARK
fb.Text="FPS: OFF" fb.TextColor3=Color3.new(0,0,0) fb.TextSize=10 fb.Font=Enum.Font.Code
fb.MouseButton1Click:Connect(function()
	fi=fi%4+1 local v=fps[fi]
	if setfpscap then setfpscap(v) end
	fb.Text=v~=60 and "FPS: "..v or "FPS: OFF"
end)
