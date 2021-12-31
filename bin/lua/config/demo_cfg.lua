local items = 
{
	{ id=1010000, name="demo1", type=1, accept_cond={1,2,3}, comp_cond=nil, reward={gold=100,diamom=50}, power=50.5, tag={{a=1,b="2"},{a=2,b="e"}},},
	{ id=1010001, name="demo2", type=1, accept_cond={1,2}, comp_cond=nil, reward={gold=100,diamom=55}, power=10.451, tag={{a=1,b="2"}},},
	{ id=1010001, name="demo3", type=1, accept_cond=nil, comp_cond=nil, reward={gold=100,diamom=55}, power=10.451, tag={{a=1,b="2"}},},
}

local idItems = 
{
	[1010000]=items[1],
	[1010001]=items[2],
	[1010001]=items[3],
}


local data = { Items = items, IdItems = idItems, reloaddata = 1 }
function data:getById(id)
    return self.IdItems[id]
end
return data
