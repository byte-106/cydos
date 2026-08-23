local PW, PH = 6, 44
local WIN = 5

local py, ay, bx, by, bvx, bvy
local ps, as_, speed
local over, msg

function reset(full)
  py = (SCREEN_H - PH) / 2
  ay = py
  ps = 0
  as_ = 0
  serve(1)
  over = false
  draw()
end

function serve(toRight)
  bx = SCREEN_W / 2
  by = SCREEN_H / 2
  speed = 130
  local d = toRight and 1 or -1
  bvx = d * speed * 0.8
  bvy = (math.random(2) * 2 - 3) * speed * 0.5
end

function draw()
  gfx.cls(COLOR.BLACK)
  for yy = 4, SCREEN_H - 4, 16 do
    gfx.fillRect(SCREEN_W / 2 - 1, yy, 2, 8, COLOR.DGRAY)
  end
  gfx.text(100, 8, tostring(ps), COLOR.WHITE, 2)
  gfx.text(206, 8, tostring(as_), COLOR.WHITE, 2)
  gfx.fillRect(10, math.floor(py), PW, PH, COLOR.WHITE)
  gfx.fillRect(SCREEN_W - 16, math.floor(ay), PW, PH, COLOR.WHITE)
  gfx.fillRect(math.floor(bx) - 3, math.floor(by) - 3, 7, 7, COLOR.YELLOW)
  if over then
    gfx.fillRect(70, 92, 180, 56, COLOR.FACE)
    gfx.rect(70, 92, 180, 56, COLOR.DGRAY)
    gfx.text(160 - math.floor(gfx.textW(msg, 1) / 2), 104, msg, COLOR.RED, 1)
    gfx.text(160 - math.floor(gfx.textW("tap to play again", 1) / 2), 124, "tap to play again", COLOR.BLACK, 1)
  end
end

function init()
  math.randomseed(millis())
  gfx.cls(COLOR.TEAL)
  gfx.text(80, 110, "First to " .. WIN .. " wins", COLOR.WHITE, 2)
  delay(900)
  reset(true)
end

function onTouch(t, x, y)
  if t == TE_PRESS or t == TE_MOVE then
    if not over and x < SCREEN_W / 2 then
      if y >= 20 then py = math.max(22, math.min(SCREEN_H - PH - 2, y - PH / 2)) end
    end
  end
  if t == TE_PRESS and over then
    reset(true)
  end
end

function loop(dt)
  if over then
    delay(20)
    return
  end
  local target = by - PH / 2 + (math.random(20) - 10)
  ay = ay + math.max(-95 * dt, math.min(95 * dt, target - ay))
  ay = math.max(22, math.min(SCREEN_H - PH - 2, ay))

  local nx = bx + bvx * dt
  local ny = by + bvy * dt

  if ny < 24 then ny = 24; bvy = -bvy; beep(700, 20) end
  if ny > SCREEN_H - 4 then ny = SCREEN_H - 4; bvy = -bvy; beep(700, 20) end

  if nx < 18 and nx > 12 and ny > py - 4 and ny < py + PH + 4 and bvx < 0 then
    bvx = -bvx
    bvy = bvy + (ny - (py + PH / 2)) * 3
    speed = speed + 8
    local m = math.sqrt(bvx * bvx + bvy * bvy)
    bvx = bvx / m * speed
    bvy = bvy / m * speed
    beep(1000, 25)
  end
  if nx > SCREEN_W - 18 and nx < SCREEN_W - 12 and ny > ay - 4 and ny < ay + PH + 4 and bvx > 0 then
    bvx = -bvx
    bvy = bvy + (ny - (ay + PH / 2)) * 3
    speed = speed + 8
    local m = math.sqrt(bvx * bvx + bvy * bvy)
    bvx = bvx / m * speed
    bvy = bvy / m * speed
    beep(900, 25)
  end

  if nx < 6 then
    as_ = as_ + 1
    beep(300, 200)
    pointEnd()
    return
  end
  if nx > SCREEN_W - 6 then
    ps = ps + 1
    beep(1500, 120)
    pointEnd()
    return
  end

  gfx.fillRect(math.floor(bx) - 3, math.floor(by) - 3, 7, 7, COLOR.BLACK)
  gfx.fillRect(10, math.floor(py), PW, PH, COLOR.WHITE)
  gfx.fillRect(SCREEN_W - 16, math.floor(ay), PW, PH, COLOR.WHITE)
  bx, by = nx, ny
  gfx.fillRect(math.floor(bx) - 3, math.floor(by) - 3, 7, 7, COLOR.YELLOW)
  delay(8)
end

function pointEnd()
  gfx.text(100, 8, tostring(ps), COLOR.WHITE, 2)
  gfx.text(206, 8, tostring(as_), COLOR.WHITE, 2)
  if ps >= WIN or as_ >= WIN then
    over = true
    if ps >= WIN then msg = "YOU WIN!" else msg = "CPU WINS" end
    draw()
  else
    serve(as_ > ps or (as_ == 0 and ps == 0))
    delay(500)
  end
end
