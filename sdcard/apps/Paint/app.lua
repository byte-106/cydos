local palette = {
  COLOR.WHITE, COLOR.YELLOW, COLOR.ORANGE, COLOR.RED,
  COLOR.GREEN, COLOR.CYAN, COLOR.BLUE, COLOR.MAGENTA,
  COLOR.LGRAY, COLOR.DGRAY, COLOR.BLACK, COLOR.FACE,
}
local PW = 25
local PY = 4
local CANVAS_Y = 30
local curCol
local lastX, lastY

function drawBar()
  gfx.fillRect(0, 0, SCREEN_W, 28, COLOR.BLACK)
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
  gfx.text(306, 8, "C", COLOR.RED, 1)
end

function init()
  gfx.cls(COLOR.WHITE)
  curCol = COLOR.BLACK
  lastX, lastY = nil, nil
  drawBar()
end

function onTouch(t, x, y)
  if t == TE_PRESS then
    if y < 28 then
      local idx = math.floor(x / PW) + 1
      if x >= 300 then
        gfx.fillRect(0, 28, SCREEN_W, SCREEN_H - 28, COLOR.WHITE)
        beep(600, 60)
        return
      end
      if idx >= 1 and idx <= #palette then
        curCol = palette[idx]
        drawBar()
        beep(1000, 20)
      end
      return
    end
    lastX, lastY = x, y
  elseif t == TE_MOVE then
    if y < 28 or lastX == nil then return end
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
