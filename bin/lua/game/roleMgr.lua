local roleMgr = {}

function roleMgr:add(role)
    self[role.id] = role
end

function roleMgr:remove(roleid)
    local role = self[roleid]
    role:clean()
    self[roleid] = nil
end

--通过roleid查找
function roleMgr:find(roleid)
    return self[roleid]
end

--通过accountid查找
function roleMgr:findby_accountid(aid)
    for k,v in pairs(self) do
        if type(v) == "table" and v.accountid == aid then
            return v
        end
    end
    return nil
end

--通过client_fd查找
function roleMgr:findby_clientfd(clientfd)
    for k,v in pairs(self) do
        if type(v) == "table" and v.clientfd == clientfd then
            return v
        end
    end
    return nil
end

--广播在线玩家
function roleMgr:broadcast(msg, msgdata)
    for k,v in pairs(self) do
        if type(v) == "table" then
            v:send(msg, msgdata)
        end
    end
end

return roleMgr