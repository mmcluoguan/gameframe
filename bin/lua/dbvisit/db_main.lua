require('strings')
AcceptMgr = require ("lua/dbvisit/acceptMgr")

function onAccept(client)
    AcceptMgr:add(client)
end

--客户端断开
function onClose(fd)
    AcceptMgr:remove(fd)
end

--处理网络数据
function onMessage(cli,msgid,msgdata,routing)
    AcceptMgr:find(cli:fd()):handle_message(msgid,msgdata,routing)
end

