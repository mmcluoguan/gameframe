require('strings')
require('lua/common/utils')
require("lua/game/define")
AcceptMgr = require ("lua/game/acceptMgr")
ConnectorMgr = require ("lua/game/connectorMgr")
RoleMgr =  require ("lua/game/roleMgr")
TimerMgr = require("lua/common/timerMgr")
BagOperator = require("lua/game/operator/bagOperator")
EmailOperator = require("lua/game/operator/emailOperator")
BaseOperator = require("lua/game/operator/baseOperator")
GmSystem = require("lua/game/system/gmSystem")
EmailSystem = require("lua/game/system/emailSystem")

--接收新客户端
function onAccept(client)
    AcceptMgr:add(client)
end

--客户端断开
function onClose(fd)
    AcceptMgr:remove(fd)
    ConnectorMgr:remove(fd)
end

--连接服务器
function onConnect(connector)
    ConnectorMgr:add(connector)
end

--处理网络数据
function onMessage(cli,msgid,msgdata,routing)
    --print("onMessage",msgid,#msgdata)
    if cli:ident() == 0 then
        --接收身份
        AcceptMgr:find(cli:fd()):handle_message(msgid,msgdata,routing)
    elseif cli:ident() == 1 then
        --连接身份
        ConnectorMgr:find(cli:fd()):handle_message(msgid,msgdata,routing)
    end    
end

local zerotimeSystem = require("lua/game/system/zerotimeSystem")
zerotimeSystem:start()

-- local co = coroutine.create(function(a, b, c)
--     for i = 1, 3 do
--         print("before coroutine yield", i)
--         print('cccccccc',type( coroutine.yield('aaa')))
--         print("after coroutine yield", i)
--     end
--     --return 'aaaaab',1,2
-- end)
-- print(coroutine.resume(co, 1, 2, 3)) --true aaa
-- print(coroutine.status(co),coroutine.running()) --suspended       thread: 0x61b000000088  true
-- print(coroutine.resume(co, 1, 2, 3))



