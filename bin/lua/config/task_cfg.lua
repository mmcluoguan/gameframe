local items = 
{
    { Id = 1010000, KeyName = "任务1", Type = 1, AcceptCond = nil, CompCond = nil, Reward = nil, },
    { Id = 1010001, KeyName = "任务2", Type = 1, AcceptCond = nil, CompCond = nil, Reward = nil, },
}

local idItems = 
{
    [1010000] = items[1],
    [1010001] = items[2],
}

local keyItems = 
{
    ["任务1"] = items[1],
    ["任务2"] = items[2],
}

local data = { Items = items, IdItems = idItems, KeyItems = keyItems, reloaddata = 1 }
function data:getById(id)
    return self.IdItems[id]
end
function data:getByKey(key)
    return self.KeyItems[key]
end
return data
