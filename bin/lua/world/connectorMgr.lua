local dbConnector =require ("lua/world/dbConnector")

--连接管理器
local connectorMgr = {}

function connectorMgr:add(client)
    local net
    if client:name() == "DbConnector" then
        net = dbConnector:new(client)
        self.dbConnector = net
    end
    self[client:fd()] = net
end

function connectorMgr:remove(fd)
    local net = self[fd]
    if net ~= nil then
      if net.name == "DbConnector" then
          self.dbConnector = nil
      end
      net:clean()
      self[fd] = nil
    end
end

function connectorMgr:find(fd)
    return self[fd]
end

return connectorMgr
