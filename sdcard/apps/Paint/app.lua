local palette = {
  COLOR.WHITE, COLOR.YELLOW, COLOR.ORANGE, COLOR.RED,
  COLOR.GREEN, COLOR.CYAN, COLOR.BLUE, COLOR.MAGENTA,
  COLOR.LGRAY, COLOR.DGRAY, COLOR.BLACK, COLOR.FACE,
}
local PW = 25
local PY = 31
local CANVAS_Y = 60
local curCol
local lastX, lastY

function drawBar()
  gfx.fillRect(0, 28, SCREEN_W, 30, COLOR.BLACK)
  for i, c in ipairs(palette) do
    local x = (i - 1) * PW
    gfx.fillRect(x + 1, PY + 1, PW - 2, 22, c)
    if c == curCol then
      gfx.rect(x, PY, PW, 24, COLOR.WHITE)
      gfx.rect(x + 1, PY + 1, PW - 2, 22, COLOR.WHITE)
    else
      gfx.rect(x, PY, PW, 24, COLOR.DGRAY)
    end
  end
  gfx.fillRect(304, PY, 14, 24, COLOR.FACE)
  gfx.text(306, 35, "C", COLOR.RED, 1)
end

function init()
  gfx.cls(COLOR.WHITE)
  curCol = COLOR.BLACK
  local raw = fs.read("paint.json")
  if raw then
    local ok, cfg = pcall(json.decode, raw)
    if ok and type(cfg) == "table" then curCol = tonumber(cfg.color) or COLOR.BLACK end
  end
  lastX, lastY = nil, nil
  drawBar()
end

function onTouch(t, x, y)
  if t == TE_PRESS then
    if y >= 28 and y < 58 then
      local idx = math.floor(x / PW) + 1
      if x >= 300 then
        gfx.fillRect(0, CANVAS_Y, SCREEN_W, SCREEN_H - CANVAS_Y, COLOR.WHITE)
        beep(600, 60)
        return
      end
      if idx >= 1 and idx <= #palette then
        curCol = palette[idx]
        fs.write("paint.json", json.encode({color = curCol}))
        drawBar()
        beep(1000, 20)
      end
      return
    end
    lastX, lastY = x, y
  elseif t == TE_MOVE then
    if y < 58 or lastX == nil then return end
  elseif t == TE_RELEASE then
    lastX, lastY = nil, nil
    return
  end
  if y >= CANVAS_Y then
    if lastX ~= nil then
      gfx.line(lastX, math.max(CANVAS_Y, lastY), x, y, curCol)
    end
    gfx.fillCircle(x, y, 2, curCol)
    lastX, lastY = x, y
  end
end

function loop(dt)
  delay(15)
end
