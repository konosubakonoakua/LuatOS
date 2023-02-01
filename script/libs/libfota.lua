--[[
@module libfota
@summary libfota fota升级
@version 1.0
@date    2023.02.01
@author  Dozingfiretruck
@usage
--注意:因使用了sys.wait()所有api需要在协程中使用
--用法实例
local libfota = require("libfota")

-- 功能:获取fota的回调函数
-- 参数:-- result：number类型，0表示成功，1表示连接失败，2表示url错误，3表示服务器断开，4表示接收报文错误
function libfota_cb(result)
    -- fota成功
    if result == 0 then
        rtos.reboot()   --如果还有其他事情要做，就不要立刻reboot
    end
end
libfota.request(libfota_cb,0,0,0,"http://iot.openluat.com/api/site/firmware_upgrade?project_key=" .. _G.PRODUCT_KEY .. "&imei=".. mobile.imei() .. "&device_key=&firmware_name=" .. _G.PROJECT.. "_LuatOS-SoC_" .. rtos.bsp() .. "&version=" .. rtos.version():sub(2) .. "." .. _G.VERSION)

]]

local sys = require "sys"
local sysplus = require "sysplus"
local libnet = require "libnet"

local libfota = {}

local taskName = "OTA_TASK"

local function netCB(msg)
    if msg[1] == socket.EVENT then
        log.info("socket网络状态变更")
    elseif msg[1] == socket.TX_OK then
        log.info("socket发送完成")
    elseif msg[1] == socket.EV_NW_RESULT_CLOSE then
        log.info("socket关闭")
    else
        log.info("未处理消息", msg[1], msg[2], msg[3], msg[4])
    end
end

local function fota_task(cbFnc,storge_location, len, param1,ota_url,ota_port,timeout)
    fota.init(storge_location, len, param1)
    local succ, param, ip, port, total, findhead, filelen, rcvCache,d1,d2,statusCode,retry,rspHead,rcvChunked,done,fotaDone,nCache
    local tbuff = zbuff.create(512)
    local rbuff = zbuff.create(4096)
    local netc = socket.create(nil, taskName)
    socket.config(netc, nil, nil, nil) -- http用的普通TCP连接
    filelen = 0
    total = 0
    retry = 0
    done = false
    rspHead = {}
    local ret = 1
    local result = libnet.waitLink(taskName, 0, netc)
    local type,host,uri = string.match(ota_url,"(%a-)://(%S-)/(%S+)")
    while retry < 3 and not done do
        if type ==nil or host ==nil then
            ret = 2
            break
        end
        result = libnet.connect(taskName, 30000, netc, host, ota_port) --后续出了http库则直接用http来处理
        tbuff:del()
        tbuff:copy(0, "GET /"..uri.."" .. " HTTP/1.1\r\n")
        tbuff:copy(nil,"Host: "..host..":"..ota_port.."\r\n")
        if filelen > 0 then --断网重连的话，只需要下载剩余的部分就行了
            tbuff:copy(nil,"Range: bytes=" .. total .. "-\r\n") 
        end
        
        tbuff:copy(nil,"Accept: application/octet-stream\r\n\r\n")
        log.info(tbuff:query())
        result = libnet.tx(taskName, 5000, netc, tbuff)
        rbuff:del()
        findhead = false
        while result do
            succ, param, ip, port = socket.rx(netc, rbuff)
            if not succ then
                log.info("服务器断开了", succ, param, ip, port)
                ret = 3
                break
            end
            if rbuff:used() > 0 then
                if findhead then
                    succ,fotaDone,nCache = fota.run(rbuff)
                    if succ then
                        total = total + rbuff:used()
                    else
                        log.error("fota写入异常，请至少在1秒后重试")
                        fota.finish(false)
                        done = true
                        break
                    end
                    log.info("收到服务器数据，长度", rbuff:used(), "fota结果", succ, done, "总共", filelen)
                    rbuff:del()
                    if fotaDone then
                        log.info("下载完成")
                        while true do
                            succ,fotaDone  = fota.isDone()
                            if fotaDone then
                                fota.finish(true)
                                log.info("FOTA完成")
                                done = true
                                ret = 0
                                break
                            end
                            sys.wait(100)
                        end
                        break
                    end
                else
                    rcvCache = rbuff:query()
                    d1,d2 = rcvCache:find("\r\n\r\n")
                    -- 打印出http response head
                    -- log.info(rcvCache:sub(1, d2))    
                    if d2 then
                        --状态行
                        _,d1,statusCode = rcvCache:find("%s(%d+)%s.-\r\n")
                        if not statusCode then
                            log.info("http没有状态返回")
                            ret = 4
                            break
                        end
                        statusCode = tonumber(statusCode)
                        if statusCode ~= 200 and statusCode ~= 206 then
                            log.info("http应答不OK", statusCode)
                            done = true
                            ret = 4
                            break
                        end
                        --应答头
                        for k,v in string.gmatch(rcvCache:sub(d1+1,d2-2),"(.-):%s*(.-)\r\n") do
                            rspHead[k] = v
                            if (string.upper(k)==string.upper("Transfer-Encoding")) and (string.upper(v)==string.upper("chunked")) then rcvChunked = true end
                        end
                        if filelen == 0 and not rcvChunked then 
                            if not rcvChunked then
                                filelen = tonumber(rspHead["Content-Length"] or "2147483647")
                            end
                        end
                        --未处理的body数据
                        rbuff:del(0, d2)
                        succ,fotaDone,nCache = fota.run(rbuff)
                        if succ then
                            total = total + rbuff:used()
                        else
                            log.error("fota写入异常，请至少在1秒后重试")
                            fota.finish(false)
                            done = true
                            break
                        end
                        log.info("收到服务器数据，长度", rbuff:used(), "fota结果", succ, done, "总共", filelen)
                        rbuff:del()

                        if fotaDone then
                            log.info("下载完成")
                            while true do
                                succ,fotaDone  = fota.isDone()
                                if fotaDone then
                                    fota.finish(true)
                                    log.info("FOTA完成")
                                    done = true
                                    ret = 0
                                    break
                                end
                                sys.wait(100)
                            end
                            break
                        end
                        
                    else
                        break
                    end
                    findhead = true
                end
            end 
            log.info("等待新数据到来")
            result, param = libnet.wait(taskName, 5000, netc)
            log.info(result, param)
            if not result then
                log.info("服务器断开了", result, param)
                break
            elseif not param then
                log.info("服务器没有数据", result, param)
                break
            end
        end
        libnet.close(taskName, 5000, netc)
        retry = retry + 1
    end
    cbFnc(ret)
    socket.release(netc)
    sysplus.taskDel(taskName)
    fota.finish(false)
end

--[[
fota升级
@api libfota.request(cbFnc,storge_location, len, param1,ota_url,ota_port,timeout)
@function cbFnc 用户回调函数，回调函数的调用形式为：cbFnc(result)
@number/string storge_location fota数据存储的起始位置<br>如果是int，则是由芯片平台具体判断<br>如果是string，则存储在文件系统中<br>如果为nil，则由底层决定存储位置
@number len 数据存储的最大空间
@userdata param1,如果数据存储在spiflash时,为spi_device
@string ota_url url
@number ota_port 请求端口,默认80
@number timeout 请求超时时间,单位毫秒,默认20000毫秒
@return nil
]]
function libfota.request(cbFnc,storge_location, len, param1,ota_url,ota_port,timeout)
    sysplus.taskInitEx(fota_task, taskName, netCB, cbFnc,storge_location, len, param1,ota_url, ota_port or 80,timeout or 20000)
end

return libfota

