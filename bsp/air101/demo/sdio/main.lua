
-- LuaTools需要PROJECT和VERSION这两个信息
PROJECT = "fatfsdemo"
VERSION = "1.0.0"

-- sys库是标配
_G.sys = require("sys")

local function fatfs_test()
    sdio.init(0)
    sdio.sd_mount(0, "/sd")
    local f = io.open("/sd/boot_time", "rb")
    local c = 0
    if f then
        local data = f:read("*a")
        log.info("fs", "data", data, data:toHex())
        c = tonumber(data)
        f:close()
    end
    log.info("fs", "boot count", c)
    c = c + 1
    f = io.open("/sd/boot_time", "wb")
    --if f ~= nil then
    log.info("fs", "write c to file", c, tostring(c))
    f:write(tostring(c))
    f:close()
    if fs then
        log.info("fsstat", fs.fsstat("/"))
        log.info("fsstat", fs.fsstat("/sd"))
    end
end

fatfs_test() -- 每次开机,把记录的数值+1

sys.taskInit(function()
    wdt.init(15000)
    sys.timerLoopStart(wdt.feed, 10000)
    while 1 do
        sys.wait(500)
    end
end)

-- 用户代码已结束---------------------------------------------
-- 结尾总是这一句
sys.run()
-- sys.run()之后后面不要加任何语句!!!!!
