require('strings')
AcceptMgr = require ("lua/world/acceptMgr")
ConnectorMgr =require ("lua/world/connectorMgr")

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
    if cli:ident() == 0 then
        --接收身份
        AcceptMgr:find(cli:fd()):handle_message(msgid,msgdata,routing)
    elseif cli:ident() == 1 then
        --连接身份
        ConnectorMgr:find(cli:fd()):handle_message(msgid,msgdata,routing)
    end    
end

