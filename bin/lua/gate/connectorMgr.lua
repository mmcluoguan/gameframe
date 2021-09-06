local dbConnector =require ("lua/gate/dbConnector")
local worldConnector =require ("lua/gate/worldConnector")
local gameConnector =require ("lua/gate/gameConnector")
local loginConnector =require ("lua/gate/loginConnector")

--连接的游戏服
local games = {}
--连接的登录服
local logins = {}
--连接管理器
local connectorMgr = {
    games = games,
    logins = logins
}

function connectorMgr:add(client)
    local net
    if client:name() == "DbConnector" then
        net = dbConnector:new(client)
        self.dbConnector = net
    elseif client:name() == "WorldConnector" then
        net = worldConnector:new(client)
        self.worldConnector = net
    elseif client:name() == "GameConnector" then
        net = gameConnector:new(client)
        self.games[client:fd()] = net
    elseif client:name() == "LoginConnector" then
        net = loginConnector:new(client)
        self.logins[client:fd()] = net
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
      elseif net.name == "GameConnector" then
          self.games[fd] = nil
      elseif net.name == "LoginConnector" then
          self.logins[fd] = nil
      end
      net:clean()
      self[fd] = nil
    end
end

function connectorMgr:find(fd)
    return self[fd]
end

return connectorMgr
