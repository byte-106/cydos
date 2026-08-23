local state = "main"
local msg = ""
local msgT = 0
local nets = {}
local selNet = nil

local ROWH = 28
local rows = { "LED Color", "Brightness", "WiFi", "Invert Colors", "Reboot", "Sleep" }

local function rowRect(i)
  return 10, 36 + (i - 1) * (ROWH + 4), 300, ROWH
end

local function backHit(x, y) return y >= 222 and x < 130 end

local function drawMain()
  gfx.cls(COLOR.TEAL)
  for i, name in ipairs(rows) do
    local x, y, w, h = rowRect(i)
    gfx.fillRect(x + 2, y + 2, w, h, COLOR.BLACK)
    gfx.fillRect(x, y, w - 2, h - 2, COLOR.FACE)
    gfx.rect(x, y, w, h, COLOR.DGRAY)
    local val = ""
    if name == "Brightness" then val = tostring(sys.getBacklight()) end
    if name == "LED Color" then
      local r, g, b = sys.getLed()
      val = r .. "," .. g .. "," .. b
    end
    if name == "Invert Colors" then
      if sys.getColorInvert() then val = "ON" else val = "OFF" end
    end
    gfx.text(x + 10, y + 7, name, COLOR.BLACK, 2)
    if val ~= "" then gfx.text(x + w - 12 - gfx.textW(val, 1), y + 10, val, COLOR.BLACK, 1) end
    gfx.text(x + w - 16, y + 8, ">", COLOR.DGRAY, 1)
  end
end

local presets = {
  { "White", 255, 255, 255 }, { "Red", 255, 0, 0 }, { "Green", 0, 255, 0 },
  { "Blue", 0, 0, 255 }, { "Yellow", 255, 255, 0 }, { "Cyan", 0, 255, 255 },
  { "Magenta", 255, 0, 255 }, { "Orange", 255, 128, 0 },
}

local function drawLed()
  gfx.cls(COLOR.FACE)
  for i, p in ipairs(presets) do
    local col = (i - 1) % 4
    local row = math.floor((i - 1) / 4)
    local x = 14 + col * 76
    local y = 40 + row * 62
    gfx.fillRect(x + 3, y + 3, 70, 52, COLOR.BLACK)
    gfx.fillRect(x, y, 70, 52, gfx.rgb565(p[2], p[3], p[4]))
    gfx.text(x + 35 - math.floor(gfx.textW(p[1], 1) / 2), y + 20, p[1], COLOR.WHITE, 1)
  end
  gfx.fillRect(14, 168, 292, 44, COLOR.FACE)
  gfx.rect(14, 168, 292, 44, COLOR.DGRAY)
  gfx.text(160 - math.floor(gfx.textW("LED OFF", 2) / 2), 181, "LED OFF", COLOR.BLACK, 2)
  gfx.text(8, 222, "< BACK", COLOR.WHITE, 2)
end

local function drawBright()
  gfx.cls(COLOR.FACE)
  local b = sys.getBacklight()
  gfx.fillRect(30, 76, 260, 40, COLOR.BLACK)
  gfx.fillRect(32, 78, math.max(2, math.floor(256 * b / 255)), 36, COLOR.YELLOW)
  gfx.rect(30, 76, 260, 40, COLOR.DGRAY)
  gfx.text(160 - math.floor(gfx.textW(tostring(b), 3) / 2), 126, tostring(b), COLOR.WHITE, 3)
  gfx.fillRect(40, 166, 100, 44, COLOR.FACE)
  gfx.rect(40, 166, 100, 44, COLOR.DGRAY)
  gfx.text(90 - 24, 179, "- 25", COLOR.BLACK, 2)
  gfx.fillRect(180, 166, 100, 44, COLOR.FACE)
  gfx.rect(180, 166, 100, 44, COLOR.DGRAY)
  gfx.text(230 - 24, 179, "+ 25", COLOR.BLACK, 2)
  gfx.text(8, 222, "< BACK", COLOR.WHITE, 2)
end

local function drawWifi()
  gfx.cls(COLOR.FACE)
  gfx.text(8, 34, wifi.status() .. "  " .. wifi.ip(), COLOR.BLACK, 1)
  gfx.fillRect(8, 46, 146, 42, COLOR.FACE)
  gfx.rect(8, 46, 146, 42, COLOR.DGRAY)
  gfx.text(81 - math.floor(gfx.textW("Scan", 2) / 2), 56, "Scan", COLOR.BLACK, 2)
  gfx.fillRect(166, 46, 146, 42, COLOR.FACE)
  gfx.rect(166, 46, 146, 42, COLOR.DGRAY)
  gfx.text(239 - math.floor(gfx.textW("Disconnect", 1) / 2), 58, "Disconnect", COLOR.BLACK, 1)
  for i, n in ipairs(nets) do
    if i > 4 then break end
    local y = 96 + (i - 1) * 31
    gfx.fillRect(8, y, 304, 28, COLOR.WHITE)
    gfx.rect(8, y, 304, 28, COLOR.DGRAY)
    local lock = ""
    if n.lock then lock = "[x]" end
    gfx.text(14, y + 10, n.ssid, COLOR.BLACK, 1)
    gfx.text(250, y + 10, lock .. " " .. n.rssi, COLOR.DGRAY, 1)
  end
  if #nets == 0 then gfx.text(60, 140, "Tap Scan to find networks", COLOR.DGRAY, 1) end
  gfx.text(8, 222, "< BACK", COLOR.WHITE, 2)
end

function init()
  drawMain()
end

function showMsg(m)
  msg = m
  msgT = millis()
  gfx.fillRect(40, 104, 240, 32, COLOR.YELLOW)
  gfx.rect(40, 104, 240, 32, COLOR.DGRAY)
  gfx.text(160 - math.floor(gfx.textW(msg, 1) / 2), 115, msg, COLOR.BLACK, 1)
end

function onTouch(t, x, y)
  if msg ~= "" and millis() - msgT < 1500 then return end
  if t ~= TE_PRESS then return end
  if state == "main" then
    for i = 1, #rows do
      local rx, ry, rw, rh = rowRect(i)
      if x >= rx and x <= rx + rw and y >= ry and y <= ry + rh then
        beep(900, 40)
        local name = rows[i]
        if name == "LED Color" then state = "led"; drawLed()
        elseif name == "Brightness" then state = "bright"; drawBright()
        elseif name == "WiFi" then state = "wifi"; nets = {}; drawWifi()
        elseif name == "Invert Colors" then
          sys.setColorInvert(not sys.getColorInvert())
          drawMain()
        elseif name == "Reboot" then sys.reboot()
        elseif name == "Sleep" then sys.sleep()
        end
      end
    end
  elseif state == "led" then
    if y < 36 or backHit(x, y) then state = "main"; drawMain(); return end
    for i, p in ipairs(presets) do
      local col = (i - 1) % 4
      local row = math.floor((i - 1) / 4)
      local px = 14 + col * 76
      local py = 40 + row * 62
      if x >= px and x <= px + 73 and y >= py and y <= py + 55 then
        sys.setLed(p[2], p[3], p[4])
        beep(1200, 40)
      end
    end
    if x >= 14 and x <= 303 and y >= 170 and y <= 210 then
      sys.ledOff()
      beep(500, 40)
    end
  elseif state == "bright" then
    if y < 36 or backHit(x, y) then state = "main"; drawMain(); return end
    if x >= 40 and x <= 137 and y >= 168 and y <= 208 then
      sys.setBacklight(math.max(10, sys.getBacklight() - 25))
    elseif x >= 180 and x <= 277 and y >= 168 and y <= 208 then
      sys.setBacklight(math.min(255, sys.getBacklight() + 25))
    end
    drawBright()
  elseif state == "wifi" then
    if y < 36 or backHit(x, y) then state = "main"; drawMain(); return end
    if x >= 8 and x <= 151 and y >= 49 and y <= 85 then
      showMsg("Scanning...")
      nets = wifi.scan() or {}
      drawWifi()
    elseif x >= 166 and x <= 309 and y >= 49 and y <= 85 then
      wifi.disconnect(true)
      nets = {}
      drawWifi()
    else
      for i, n in ipairs(nets) do
        if i > 4 then break end
        local ny = 96 + (i - 1) * 31
        if y >= ny and y <= ny + 28 then
          local pwd = ""
          if n.lock then
            pwd = kb.input("Password:", true)
            if pwd == nil then return end
          end
          showMsg("Connecting...")
          if wifi.connect(n.ssid, pwd) then
            showMsg("Connected: " .. wifi.ip())
          else
            showMsg("Failed!")
          end
          delay(800)
          drawWifi()
        end
      end
    end
  end
end

function loop(dt)
  if msg ~= "" and millis() - msgT > 1500 then
    msg = ""
    if state == "wifi" then drawWifi() else drawMain() end
  end
  delay(30)
end
