local disp, acc, op, fresh
local keys = {
  { "7", "8", "9", "/" },
  { "4", "5", "6", "*" },
  { "1", "2", "3", "-" },
  { "C", "0", "=", "+" },
}
local KX, KY = 8, 76
local KW, KH, GAP = 74, 36, 3

function drawDisp()
  gfx.fillRect(8, 34, 304, 36, COLOR.WHITE)
  gfx.rect(8, 34, 304, 36, COLOR.DGRAY)
  local s = disp
  if #s > 20 then s = string.sub(s, #s - 19) end
  gfx.text(300 - gfx.textW(s, 2) - 8, 45, s, COLOR.BLACK, 2)
end

function drawKeys()
  for r = 1, 4 do
    for c = 1, 4 do
      local x = KX + (c - 1) * (KW + GAP)
      local y = KY + (r - 1) * (KH + GAP)
      gfx.fillRect(x + 2, y + 2, KW - 2, KH - 2, COLOR.BLACK)
      gfx.fillRect(x, y, KW - 2, KH - 2, COLOR.FACE)
      gfx.rect(x, y, KW, KH, COLOR.DGRAY)
      gfx.text(x + KW / 2 - 5, y + 11, keys[r][c], COLOR.BLACK, 2)
    end
  end
end

function init()
  disp = "0"
  acc = nil
  op = nil
  fresh = true
  gfx.cls(COLOR.TEAL)
  drawDisp()
  drawKeys()
end

function apply(a, o, b)
  if o == "+" then return a + b end
  if o == "-" then return a - b end
  if o == "*" then return a * b end
  if o == "/" then
    if b == 0 then return nil end
    return a / b
  end
  return b
end

function fmt(n)
  if n == nil then return "Error" end
  if math.floor(n) == n and math.abs(n) < 1e9 then
    return tostring(math.floor(n))
  end
  local s = string.format("%.6f", n)
  s = string.gsub(s, "0+$", "")
  s = string.gsub(s, "%.$", "")
  return s
end

function press(k)
  beep(1100, 25)
  if k >= "0" and k <= "9" then
    if fresh or disp == "Error" then disp = k; fresh = false
    elseif #disp < 18 then disp = disp .. k
    end
    if disp ~= "0" or fresh then drawDisp() else drawDisp() end
  elseif k == "C" then
    disp = "0"; acc = nil; op = nil; fresh = true
    drawDisp()
  elseif k == "+" or k == "-" or k == "*" or k == "/" then
    local v = tonumber(disp)
    if acc ~= nil and op ~= nil and not fresh then
      acc = apply(acc, op, v)
      disp = fmt(acc)
    else
      acc = v
    end
    op = k
    fresh = true
    drawDisp()
  elseif k == "=" then
    if acc ~= nil and op ~= nil then
      local r = apply(acc, op, tonumber(disp))
      disp = fmt(r)
      acc = nil
      op = nil
      fresh = true
      drawDisp()
    end
  end
end

function onTouch(t, x, y)
  if t ~= TE_PRESS then return end
  for r = 1, 4 do
    for c = 1, 4 do
      local kx = KX + (c - 1) * (KW + GAP)
      local ky = KY + (r - 1) * (KH + GAP)
      if x >= kx and x <= kx + KW and y >= ky and y <= ky + KH then
        press(keys[r][c])
        return
      end
    end
  end
end

function loop(dt)
  delay(30)
end
