local COLS, ROWS = 6, 3
local AW, AH, AGAP = 30, 20, 8
local GX = (SCREEN_W - COLS * (AW + AGAP)) / 2
local GY = 40

local ship_x, aliens, adir, aspeed, astep, acc
local bullets, bombs, score, lives, wave
local over, msg

function resetWave()
  aliens = {}
  for r = 0, ROWS - 1 do
    for c = 0, COLS - 1 do
      table.insert(aliens, { c * (AW + AGAP), r * (AH + 10) })
    end
  end
  adir = 1
  aspeed = 14 + wave * 6
  astep = 0
  bullets = {}
  bombs = {}
end

function init()
  math.randomseed(millis())
  score = 0
  lives = 3
  wave = 1
  ship_x = SCREEN_W / 2
  over = false
  resetWave()
  draw()
end

function draw()
  gfx.cls(COLOR.BLACK)
  gfx.text(8, 4, "SCORE " .. score, COLOR.WHITE, 1)
  gfx.text(250, 4, "LIVES " .. lives, COLOR.WHITE, 1)
  for _, a in ipairs(aliens) do
    local ax = GX + a[1] + astep
    local ay = GY + a[2]
    gfx.fillRect(ax + 6, ay, AW - 12, 4, COLOR.MAGENTA)
    gfx.fillRect(ax, ay + 4, AW, AH - 8, COLOR.MAGENTA)
    gfx.fillRect(ax + 2, ay + AH - 4, AW - 4, 4, COLOR.MAGENTA)
    gfx.fillRect(ax + 5, ay + 7, 4, 4, COLOR.BLACK)
    gfx.fillRect(ax + AW - 9, ay + 7, 4, 4, COLOR.BLACK)
  end
  gfx.fillRect(math.floor(ship_x) - 12, 208, 24, 8, COLOR.CYAN)
  gfx.fillRect(math.floor(ship_x) - 4, 202, 8, 8, COLOR.CYAN)
  for _, b in ipairs(bullets) do
    gfx.fillRect(b[1], b[2], 3, 8, COLOR.YELLOW)
  end
  for _, b in ipairs(bombs) do
    gfx.fillRect(b[1], b[2], 3, 8, COLOR.RED)
  end
  if over then
    gfx.fillRect(60, 90, 200, 60, COLOR.FACE)
    gfx.rect(60, 90, 200, 60, COLOR.DGRAY)
    gfx.text(160 - math.floor(gfx.textW(msg, 2) / 2), 100, msg, COLOR.RED, 2)
    gfx.text(160 - math.floor(gfx.textW("tap to retry", 1) / 2), 126, "tap to retry", COLOR.BLACK, 1)
  end
end

function onTouch(t, x, y)
  if over then
    if t == TE_PRESS then init() end
    return
  end
  if (t == TE_PRESS or t == TE_MOVE) and y > 180 then
    ship_x = math.max(16, math.min(SCREEN_W - 16, x))
  elseif t == TE_PRESS and y <= 180 then
    if #bullets < 3 then
      table.insert(bullets, { math.floor(ship_x) - 1, 198 })
      beep(1600, 25)
    end
  end
end

function loop(dt)
  if over then delay(20); return end
  local down = false
  local minX, maxX = 9999, -9999
  for _, a in ipairs(aliens) do
    local ax = a[1] + astep
    if ax < minX then minX = ax end
    if ax + AW > maxX then maxX = ax + AW end
  end
  if maxX >= SCREEN_W - 4 and adir == 1 or minX <= 4 and adir == -1 then
    adir = -adir
    down = true
  end
  astep = astep + adir * aspeed * dt
  if down then
    for _, a in ipairs(aliens) do a[2] = a[2] + 14 end
  end

  for i = #bullets, 1, -1 do
    local b = bullets[i]
    b[2] = b[2] - 260 * dt
    if b[2] < GY - 10 then
      table.remove(bullets, i)
    else
      for j = #aliens, 1, -1 do
        local a = aliens[j]
        local ax = GX + a[1] + astep
        local ay = GY + a[2]
        if b[1] + 3 > ax and b[1] < ax + AW and b[2] < ay + AH and b[2] + 8 > ay then
          table.remove(aliens, j)
          table.remove(bullets, i)
          score = score + 10
          beep(500, 50)
          break
        end
      end
    end
  end

  if #aliens == 0 then
    wave = wave + 1
    msg = nil
    gfx.cls(COLOR.TEAL)
    gfx.text(110, 110, "WAVE " .. wave, COLOR.WHITE, 3)
    delay(900)
    resetWave()
    draw()
    return
  end

  if math.random(100) < 2 + wave then
    local a = aliens[math.random(#aliens)]
    table.insert(bombs, { GX + a[1] + astep + AW / 2, GY + a[2] + AH })
  end
  for i = #bombs, 1, -1 do
    local b = bombs[i]
    b[2] = b[2] + 120 * dt
    if b[2] > SCREEN_H then
      table.remove(bombs, i)
    elseif b[2] + 8 > 202 and b[1] + 3 > ship_x - 12 and b[1] < ship_x + 12 then
      table.remove(bombs, i)
      lives = lives - 1
      beep(200, 300)
      if lives <= 0 then
        over = true
        msg = "GAME OVER"
        sys.set("invaders_best", tostring(score))
        draw()
        return
      end
      draw()
    end
  end

  for _, a in ipairs(aliens) do
    if GY + a[2] + AH > 200 then
      over = true
      msg = "INVADED!"
      draw()
      return
    end
  end

  draw()
  delay(15)
end
