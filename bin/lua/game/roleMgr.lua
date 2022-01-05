local roleMgr = {
    rid={},
    aid={},
    cfd={}
}

function roleMgr:add(role)
    self.rid[role.id] = role
    self.aid[role.accountid] = role
    self.cfd[role.clientfd] = role
end

function roleMgr:remove(roleid)
    local role = self.rid[roleid]
    local aid = role.accountid
    local clientfd = role.clientfd
    role:clean()
    self.rid[roleid] = nil
    self.aid[aid] = nil
    self.cfd[clientfd] = nil
end

--通过roleid查找
function roleMgr:find(roleid)
    return self.rid[roleid]
end

--通过accountid查找
function roleMgr:findby_accountid(aid)
    return self.aid[aid]
end

--通过client_fd查找
function roleMgr:findby_clientfd(clientfd)
    return self.cfd[clientfd]
end

--广播在线玩家
function roleMgr:broadcast(msg, msgdata)
    for k,v in pairs(self.rid) do
        v:send(msg, msgdata)
    end
end

return roleMgr