local CW, CH = 12, 12
local COLS, ROWS = 26, 13
local OX, OY = 4, 34
local CTRL_Y = 164

local snake, dir, food, score, alive, stepMs, acc
local best = 0
do
  local raw = fs.read("snake.json")
  if raw then
    local ok, cfg = pcall(json.decode, raw)
    if ok and type(cfg) == "table" then best = tonumber(cfg.best) or 0 end
  end
end

local btns = {
  { 132, 172, "L" }, { 196, 172, "R" },
}

local function spawnFood()
  while true do
    local fx = math.random(0, COLS - 1)
    local fy = math.random(0, ROWS - 1)
    local ok = true
    for _, s in ipairs(snake) do
      if s[1] == fx and s[2] == fy then ok = false; break end
    end
    if ok then food = { fx, fy }; return end
  end
end

local function drawCell(cx, cy, c)
  gfx.fillRect(OX + cx * CW + 1, OY + cy * CH + 1, CW - 2, CH - 2, c)
end

local function field()
  return OX, OY, COLS * CW, ROWS * CH
end

function init()
  math.randomseed(millis())
  gfx.cls(COLOR.TEAL)
  gfx.text(8, 226, "SCORE:0", COLOR.WHITE, 1)
  gfx.text(250, 226, "BEST:" .. best, COLOR.WHITE, 1)
  snake = { { 8, 6 }, { 7, 6 }, { 6, 6 } }
  dir = { 1, 0 }
  score = 0
  alive = true
  stepMs = 160
  acc = 0
  local fx, fy, fw, fh = field()
  gfx.fillRect(fx, fy, fw, fh, COLOR.BLACK)
  spawnFood()
end

function gameOver()
  alive = false
  if score > best then
    best = score
    fs.write("snake.json", json.encode({best = best}))
  end
  gfx.fillRect(60, 60, 200, 70, COLOR.FACE)
  gfx.rect(60, 60, 200, 70, COLOR.DGRAY)
  gfx.rect(61, 61, 198, 68, COLOR.WHITE)
  gfx.text(160 - math.floor(gfx.textW("GAME OVER", 2) / 2), 72, "GAME OVER", COLOR.RED, 2)
  gfx.text(160 - math.floor(gfx.textW("Score " .. score .. " - tap to retry", 1) / 2), 104, "Score " .. score .. " - tap to retry", COLOR.BLACK, 1)
end

function step()
  local head = { snake[1][1] + dir[1], snake[1][2] + dir[2] }
  if head[1] < 0 or head[1] >= COLS or head[2] < 0 or head[2] >= ROWS then
    return gameOver()
  end
  for _, s in ipairs(snake) do
    if s[1] == head[1] and s[2] == head[2] then return gameOver() end
  end
  table.insert(snake, 1, head)
  drawCell(head[1], head[2], COLOR.GREEN)
  if head[1] == food[1] and head[2] == food[2] then
    score = score + 10
    gfx.fillCircle(OX + food[1] * CW + CW / 2, OY + food[2] * CH + CH / 2, 5, COLOR.BLACK)
    gfx.text(8, 226, "SCORE:" .. score, COLOR.WHITE, 1)
    beep(1400, 30)
    if stepMs > 70 then stepMs = stepMs - 3 end
    spawnFood()
  else
    local tail = table.remove(snake)
    drawCell(tail[1], tail[2], COLOR.BLACK)
  end
  drawCell(food[1], food[2], COLOR.RED)
end

function onTouch(t, x, y)
  if not alive then
    if t == TE_PRESS then init() end
    return
  end
  if t ~= TE_PRESS then return end
  local fx, fy, fw, fh = field()
  if y < CTRL_Y and (x < fx or x > fx + fw or y < fy or y > fy + fh) and y < 40 then return end
  if y >= CTRL_Y then
    if x >= 132 and x <= 188 and y >= 172 and y <= 228 then
      if dir[1] ~= 1 then dir = { -1, 0 } end
    elseif x >= 196 and x <= 252 and y >= 172 and y <= 228 then
      if dir[1] ~= -1 then dir = { 1, 0 } end
    elseif x < 120 then
      if dir[2] ~= 1 then dir = { 0, -1 } end
    elseif x > 260 then
      if dir[2] ~= -1 then dir = { 0, 1 } end
    end
  else
    local hx, hy = snake[1][1], snake[1][2]
    if math.abs(x - (OX + hx * CW)) > math.abs(y - (OY + hy * CH)) then
      local nd = x > OX + hx * CW and { 1, 0 } or { -1, 0 }
      if nd[1] ~= -dir[1] then dir = nd end
    else
      local nd = y > OY + hy * CH and { 0, 1 } or { 0, -1 }
      if nd[2] ~= -dir[2] then dir = nd end
    end
  end
end

function loop(dt)
  if alive then
    acc = acc + dt
    while acc >= stepMs do
      acc = acc - stepMs
      step()
      if not alive then break end
    end
  end
  delay(10)
end
