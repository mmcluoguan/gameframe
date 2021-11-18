local items = 
{
    { Id = 1000100, KeyName = "1级碎片", Overlap = 99, Type = 1, },
    { Id = 1000101, KeyName = "2级碎片", Overlap = 99, Type = 1, },
}

local idItems = 
{
    [1000100] = items[1],
    [1000101] = items[2],
}

local keyItems = 
{
    ["1级碎片"] = items[1],
    ["2级碎片"] = items[2],
}

local data = { Items = items, IdItems = idItems, KeyItems = keyItems, reloaddata = 1 }
function data:getById(id)
    return self.IdItems[id]
end
function data:getByKey(key)
    return self.KeyItems[key]
end
return data
