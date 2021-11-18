local dbConnector =require ("lua/game/dbConnector")
local worldConnector =require ("lua/game/worldConnector")

--连接管理器
local connectorMgr = {
    dbConnectors={},
    worldConnectors={},
}

function connectorMgr:add(client)
    local net
    if client:name() == "DbConnector" then
        net = dbConnector:new(client)
        table.insert(self.dbConnectors,#self.dbConnectors+1,net)
        net.pos = #self.dbConnectors
        EmailSystem:load()
    elseif client:name() == "WorldConnector" then
        net = worldConnector:new(client)
        table.insert(self.worldConnectors,#self.worldConnectors+1,net)
        net.pos = #self.worldConnectors
    end
    self[client:fd()] = net
end

function connectorMgr:remove(fd)
    local net = self[fd]
    if net ~= nil then
      if net.name == "DbConnector" then
          table.remove(self.dbConnectors,net.pos)
      elseif net.name == "WorldConnector" then
          table.remove(self.worldConnectors,net.pos)
      end
      net:clean()
      self[fd] = nil
    end
end

function connectorMgr:find(fd)
    return self[fd]
end

function connectorMgr:dbConnector()
    return self.dbConnectors[1]
end

function connectorMgr:worldConnector()
    return self.worldConnectors[1]
end

return connectorMgr
