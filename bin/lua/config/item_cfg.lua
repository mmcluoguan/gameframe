local items =
{
	{ id=1000100, name="1级碎片", type=1, overlap=99,},
	{ id=1010001, name="2级碎片", type=1, overlap=99,},
}

local idItems =
{
	[1000100]=items[1],
	[1010001]=items[2],
}


local data = { Items = items, IdItems = idItems, reloaddata = 1 }
function data:getById(id)
    return self.IdItems[id]
end
return data
