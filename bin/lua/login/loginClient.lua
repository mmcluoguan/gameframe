local baseNet = require ("lua/common/baseNet")
ConnectorMgr = require ("lua/login/connectorMgr")

local loginClient = {}
--继承
setmetatable(loginClient,baseNet)
baseNet.__index = baseNet

function loginClient:init()
    baseNet:init(self);
end

function loginClient:clean()
    baseNet:clean(self);
end

return loginClient;