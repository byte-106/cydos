# How to make a CYDOS app

An app is a **folder** on the SD card under `/apps` containing `app.lua`.
That's it.

```
/apps
  /Hello
    app.lua         <- required, Lua code
    manifest.txt    <- optional, first line = name shown on desktop
    icon.jpg        <- optional, 48x48 JPEG shown on the desktop grid
```

Copy the folder onto the card (or edit it right in the CYD with the built-in
Editor), reboot, and your app appears on the desktop.

## Two ways to structure an app

**Event mode** - if you define `init`, CYDOS calls it once, then calls
`loop(dt)` repeatedly (dt = seconds since last frame) and `onTouch(type,x,y)`
on touch input:

```lua
function init()
  gfx.cls(COLOR.NAVY)
  gfx.text(10, 10, "Hello!", COLOR.WHITE, 2)
end

function onTouch(t, x, y)
  if t == TE_PRESS then beep(1000, 50) end
end

function loop(dt)
  delay(30)
end
```

**Free-run mode** - no functions defined? Your script just runs top to
bottom. Call `delay()` regularly (it pumps the touchscreen and feeds the
watchdog):

```lua
gfx.cls(COLOR.BLACK)
for i = 1, 20 do
  gfx.fillCircle(math.random(0, SCREEN_W), math.random(0, SCREEN_H), 6,
                 COLOR[{"RED","GREEN","YELLOW"}[math.random(3)]])
  delay(100)
end
```

If neither style yields within ~4 s, the watchdog kills the app back to the
desktop.

The titlebar X button always works and closes your app.

## API reference

### Drawing - `gfx.*`

| call | meaning |
|---|---|
| `cls([color])` | clear screen |
| `px(x,y,c)` | pixel |
| `line(x0,y0,x1,y1,c)` | line |
| `rect(x,y,w,h,c)` | outline rect |
| `fillRect(x,y,w,h,c)` | filled rect |
| `circle(x,y,r,c[,filled])` | circle |
| `fillCircle(x,y,r,c)` | filled circle |
| `text(x,y,s,[c],[size])` | draw text (returns width in px); size 1-6 |
| `textW(s,[size])` | measure text width |
| `rgb565(r,g,b)` | pack a color |
| `drawJpg(path,x,y,[scale])` | draw JPEG from SD (path relative to your app dir) |

### Colors

`COLOR.BLACK WHITE RED GREEN BLUE YELLOW CYAN MAGENTA ORANGE GRAY DGRAY
LGRAY TEAL NAVY FACE` are available both as `COLOR.X` and as plain globals
(`WHITE`). All are RGB565 ints you can pass anywhere a color is expected.

`TEAL`, `NAVY` and `FACE` follow the active system theme, so apps that use
them blend in on every CYDOS version. Hardcoding raw colors makes your app
look the same everywhere.
Constants: `SCREEN_W` (320), `SCREEN_H` (240), `TE_PRESS TE_MOVE TE_RELEASE`.

The shell draws a title bar over the top 28 px of the screen while your app
runs - keep your UI below `y = 30` and it will never fight the chrome.

### Touch

`onTouch(t,x,y)` receives `t` = `TE_PRESS`(1), `TE_MOVE`(2) or
`TE_RELEASE`(3). In free-run mode use:

```lua
local t, x, y = pollEvent()
if t == TE_PRESS then ... end
```

### Files - `fs.*`

Sandboxed to `/data/<YourAppName>/` - you cannot read other apps' files or
the system:

`read(name)` `write(name,data)` `append(name,data)` `exists(name)`
`delete(name)` `ls()` -> table of names. Missing reads return nil.

### JSON - `json.*`

Every app gets a `json` global for saving structured config in its sandbox:

```lua
-- load with a default
local cfg = {best = 0, color = "blue"}
local raw = fs.read("config.json")
if raw then
  local ok, saved = pcall(json.decode, raw)
  if ok and type(saved) == "table" then
    for k, v in pairs(saved) do cfg[k] = v end
  end
end

-- save
cfg.best = 42
fs.write("config.json", json.encode(cfg))
```

`json.encode(t)` -> string, `json.decode(s)` -> table (objects, arrays,
strings, numbers, booleans, null). Users can open these files from the
Files app or a card reader to peek at or hand-edit app data.

The system keeps its own human-editable settings file too:
`/data/Settings/settings.json` (brightness, LED color, `wifi_ssid` and
`wifi_pass` slots...). It is applied on boot and re-synced whenever you
change settings - fill in the WiFi lines on a card reader, reboot, done.

### System - `sys.*`

```lua
sys.setBacklight(0..255)   sys.getBacklight()
sys.setLed(r,g,b)          sys.getLed() -> r,g,b     sys.ledOff()
sys.setColorInvert(bool)   sys.getColorInvert()
sys.set(key,val)           -- global key/value store (survives reboot)
sys.get(key,[default])
sys.reboot()               sys.sleep()  -- deep sleep; BOOT wakes it
```

### Network - `wifi.*`, `net.*`

```lua
local list = wifi.scan()      -- {{ssid=,rssi=,lock=},...}
local ok   = wifi.connect(ssid, password)   -- blocks until result
wifi.disconnect()
wifi.status()                 -- human-readable state string
wifi.ip()                     -- "192.168.1.42" or ""
net.time()                    -- {year,month,day,hour,min,sec,wday} or nil
local body, err = net.get("http://example.com/api")
```

### Misc

`kb.input(title,[mask])` -> typed string or nil (cancel). On-screen keyboard.
`beep(freq,ms)` `millis()` `delay(ms)` `exitApp()`
Globals: `VERSION`, `APP_NAME`, `APP_DIR` (`/apps/YourApp`), `DATA_DIR`.

Lua is 5.5 with base/table/string/math libraries only - there is
intentionally no `io`, `os`, `debug` or `package`. Use `fs.*`, `sys.*`,
`millis()` instead.

## Tips

- The screen is 320x240, rotated landscape, touched with fingers: make hit
  targets at least 40 px.
- Redraw only what changed inside `loop`; full-screen redraws cost ~40 ms.
- Store high scores in your own `config.json` with the `json` global (see
  JSON section above).
- Test on the device often: the Editor app can edit `/apps/.../app.lua`
  directly on the card, then relaunch.
