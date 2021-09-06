local baseNet = require ("lua/common/baseNet")

local dbClient = {}
--继承
setmetatable(dbClient,baseNet)
baseNet.__index = baseNet

function dbClient:init()
    baseNet:init(self);
end

function dbClient:clean()
    baseNet:clean(self);
end

function dbClient:regMsg()
    baseNet:regMsg(self);
end

return dbClient;