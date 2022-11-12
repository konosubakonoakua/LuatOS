local libnet = require "libnet"

sys.taskInit(
    function()
        while 1 do
            log.info("mem.lua", rtos.meminfo())
            log.info("mem.sys", rtos.meminfo("sys"))
            local code, headers, body = http2.request("GET","http://site0.cn/api/httptest/simple/time").wait()
            log.info("http2.get", code, json.encode(headers), body)
            sys.wait(1000)
        end
    end
)
