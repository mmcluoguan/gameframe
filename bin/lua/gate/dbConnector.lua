local baseNet =require ("lua/common/baseNet")

local dbConnector = {}
--继承
setmetatable(dbConnector,baseNet)
baseNet.__index = baseNet

function dbConnector:init()
    baseNet:init(self);
end

function dbConnector:clean()
    baseNet:clean(self);
end

return dbConnector;