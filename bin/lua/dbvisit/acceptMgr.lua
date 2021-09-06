local dbClient =require ("lua/dbvisit/dbClient")

--接收管理器
local acceptMgr = {}

function acceptMgr:add(client)
    local net = dbClient:new(client)
    self[client:fd()] = net
end

function acceptMgr:remove(fd)
    local net = self[fd]
    if net ~= nil then
      net:clean()
      self[fd] = nil
    end
end

function acceptMgr:find(fd)
    return self[fd]
end

return acceptMgr
