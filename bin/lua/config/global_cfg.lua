local items = 
{
    { Id = 1, KeyName = "每日抽奖次数", Value = 10, },
}

local idItems = 
{
    [1] = items[1],
}

local keyItems = 
{
    ["每日抽奖次数"] = items[1],
}

local data = { Items = items, IdItems = idItems, KeyItems = keyItems, reloaddata = 1 }
function data:getById(id)
    return self.IdItems[id]
end
function data:getByKey(key)
    return self.KeyItems[key]
end
return data
