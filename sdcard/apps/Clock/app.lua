local days = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" }
local months = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" }
local lastSec, lastDraw

function pad(n)
  if n < 10 then return "0" .. n end
  return tostring(n)
end

function drawClock()
  gfx.cls(COLOR.BLACK)
  local t = net.time()
  if t == nil then
    gfx.text(160 - math.floor(gfx.textW("NO TIME SYNC", 2) / 2), 90, "NO TIME SYNC", COLOR.ORANGE, 2)
    gfx.text(160 - math.floor(gfx.textW("Connect WiFi in Settings", 1) / 2), 126, "Connect WiFi in Settings", COLOR.LGRAY, 1)
    gfx.text(160 - math.floor(gfx.textW(wifi.status(), 1) / 2), 150, wifi.status(), COLOR.DGRAY, 1)
  else
    local hs = pad(t.hour) .. ":" .. pad(t.min)
    local ss = ":" .. pad(t.sec)
    local total = gfx.textW(hs, 6) + 4 + gfx.textW(ss, 3)
    local x0 = math.floor((SCREEN_W - total) / 2)
    gfx.text(x0, 48, hs, COLOR.WHITE, 6)
    gfx.text(x0 + gfx.textW(hs, 6) + 4, 72, ss, COLOR.YELLOW, 3)
    local ds = days[t.wday + 1] .. ", " .. months[t.month] .. " " .. t.day .. " " .. t.year
    gfx.text(160 - math.floor(gfx.textW(ds, 2) / 2), 156, ds, COLOR.CYAN, 2)
    gfx.text(8, 226, wifi.ssid(), COLOR.DGRAY, 1)
  end
end

function init()
  lastSec = -1
  drawClock()
end

function onTouch(t, x, y) end

function loop(dt)
  local t = net.time()
  local s = t and t.sec or 0
  if s ~= lastSec then
    lastSec = s
    drawClock()
  end
  delay(200)
end
