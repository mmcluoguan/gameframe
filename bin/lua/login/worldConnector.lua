local baseNet =require ("lua/common/baseNet")

local worldConnector = {}
--继承
setmetatable(worldConnector,baseNet)
baseNet.__index = baseNet

function worldConnector:init()
    baseNet:init(self);
end

function worldConnector:clean()
    baseNet:clean(self);
end

return worldConnector;