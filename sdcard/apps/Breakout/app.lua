local COLS, ROWS = 8, 4
local BW, BH = 38, 14
local OX = (SCREEN_W - COLS * BW) / 2
local OY = 40

local bricks, ball, vel, px, lives, score
local over, msg, won

function buildBricks()
  bricks = {}
  local cols = { COLOR.RED, COLOR.ORANGE, COLOR.GREEN, COLOR.CYAN }
  for r = 0, ROWS - 1 do
    for c = 0, COLS - 1 do
      table.insert(bricks, { OX + c * BW + 1, OY + r * (BH + 3), c * BW + 1, r * (BH + 3), cols[r + 1] })
      gfx.fillRect(OX + c * BW + 1, OY + r * (BH + 3), BW - 2, BH, cols[r + 1])
    end
  end
end

function serve()
  ball = { SCREEN_W / 2, 150 }
  local a = math.rad(60 + math.random(60))
  local sp = 150
  if math.random(2) == 1 then a = math.pi - a end
  vel = { sp * math.cos(a), -sp * math.sin(a) }
end

function init()
  math.randomseed(millis())
  gfx.cls(COLOR.BLACK)
  buildBricks()
  px = SCREEN_W / 2
  lives = 3
  score = 0
  over = false
  won = false
  drawPaddle(SCREEN_W / 2)
  hud()
  serve()
end

function hud()
  gfx.fillRect(0, 0, SCREEN_W, 18, COLOR.BLACK)
  gfx.text(8, 4, "SCORE " .. score, COLOR.WHITE, 1)
  gfx.text(250, 4, "LIVES " .. lives, COLOR.WHITE, 1)
end

function drawPaddle(newx)
  gfx.fillRect(math.floor(px) - 24, 222, 48, 6, COLOR.BLACK)
  px = newx
  gfx.fillRect(math.floor(px) - 24, 222, 48, 6, COLOR.WHITE)
end

function onTouch(t, x, y)
  if over then
    if t == TE_PRESS then init() end
    return
  end
  if t == TE_PRESS or t == TE_MOVE then
    drawPaddle(math.max(26, math.min(SCREEN_W - 26, x)))
  end
end

function eraseBall()
  gfx.fillRect(math.floor(ball[1]) - 3, math.floor(ball[2]) - 3, 7, 7, COLOR.BLACK)
end

function showEnd()
  over = true
  gfx.fillRect(60, 90, 200, 60, COLOR.FACE)
  gfx.rect(60, 90, 200, 60, COLOR.DGRAY)
  gfx.text(160 - math.floor(gfx.textW(msg, 2) / 2), 98, msg, won and COLOR.GREEN or COLOR.RED, 2)
  gfx.text(160 - math.floor(gfx.textW("tap to retry", 1) / 2), 126, "tap to retry", COLOR.BLACK, 1)
end

function loop(dt)
  if over then delay(20); return end

  local nx = ball[1] + vel[1] * dt
  local ny = ball[2] + vel[2] * dt

  if nx < 5 then nx = 5; vel[1] = -vel[1]; beep(700, 15) end
  if nx > SCREEN_W - 5 then nx = SCREEN_W - 5; vel[1] = -vel[1]; beep(700, 15) end
  if ny < 20 then ny = 20; vel[2] = -vel[2]; beep(700, 15) end

  if ny > 216 and ny < 226 and vel[2] > 0 and math.abs(nx - px) < 28 then
    vel[2] = -math.abs(vel[2])
    vel[1] = vel[1] + (nx - px) * 4
    local m = math.sqrt(vel[1] ^ 2 + vel[2] ^ 2)
    if m > 0 then
      vel[1] = vel[1] / m * 165
      vel[2] = vel[2] / m * 165
    end
    beep(1100, 20)
  end

  for i = #bricks, 1, -1 do
    local b = bricks[i]
    if nx + 3 > b[1] and nx - 3 < b[1] + BW - 2 and ny + 3 > b[2] and ny - 3 < b[2] + BH then
      gfx.fillRect(b[1], b[2], BW - 2, BH, COLOR.BLACK)
      table.remove(bricks, i)
      score = score + 5
      hud()
      local fromSide = nx < b[1] or nx > b[1] + BW - 2
      if fromSide then vel[1] = -vel[1] else vel[2] = -vel[2] end
      beep(1300, 20)
      break
    end
  end

  if #bricks == 0 then
    won = true
    msg = "YOU CLEARED IT!"
    sys.set("breakout_best", tostring(score))
    showEnd()
    return
  end

  if ny > SCREEN_H + 6 then
    lives = lives - 1
    beep(250, 300)
    hud()
    if lives <= 0 then
      won = false
      msg = "GAME OVER"
      showEnd()
      return
    end
    serve()
    return
  end

  eraseBall()
  ball[1], ball[2] = nx, ny
  gfx.fillRect(math.floor(nx) - 3, math.floor(ny) - 3, 7, 7, COLOR.YELLOW)
  delay(8)
end
