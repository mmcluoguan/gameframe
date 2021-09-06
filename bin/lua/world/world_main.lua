local acceptMgr = require ("lua/world/acceptMgr")
local connectorMgr =require ("lua/world/connectorMgr")

function onAccept(client)
    acceptMgr:add(client)
end

--客户端断开
function onClose(fd)
    acceptMgr:remove(fd)
    connectorMgr:remove(fd)
end

--连接服务器
function onConnect(connector)
    connectorMgr:add(connector)
end

--处理网络数据
function onMessage(cli,msgid,msgdata)
    if cli:ident() == 0 then
        --接收身份
        acceptMgr:find(cli:fd()):handle_message(msgid,msgdata,routing)
    elseif cli:ident() == 1 then
        --连接身份
        connectorMgr:find(cli:fd()):handle_message(msgid,msgdata,routing)
    end    
end

