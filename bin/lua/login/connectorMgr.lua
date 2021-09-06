local dbConnector =require ("lua/login/dbConnector")
local worldConnector =require ("lua/login/worldConnector")

--连接管理器
local connectorMgr = {}

function connectorMgr:add(client)
    local net
    if client:name() == "DbConnector" then
        net = dbConnector:new(client)
        self.dbConnector = net
    elseif client:name() == "WorldConnector" then
        net = worldConnector:new(client)
        self.worldConnector = net
    end
    self[client:fd()] = net
end

function connectorMgr:remove(fd)
    local net = self[fd]
    if net ~= nil then
      if net.name == "DbConnector" then
          self.dbConnector = nil
      elseif net.name == "WorldConnector" then
          self.worldConnector = nil
      end
      net:clean()
      self[fd] = nil
    end
end

function connectorMgr:find(fd)
    return self[fd]
end

return connectorMgr
