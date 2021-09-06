local acceptMgr = require ("lua/dbvisit/acceptMgr")

function onAccept(client)
    acceptMgr:add(client)
end

--客户端断开
function onClose(fd)
    acceptMgr:remove(fd)
end

--处理网络数据
function onMessage(cli,msgid,msgdata)
    acceptMgr:find(cli:fd()):handle_message(msgid,msgdata,routing)
end

