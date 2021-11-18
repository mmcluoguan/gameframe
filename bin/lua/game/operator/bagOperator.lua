--背包操作
local item_cfg = require("lua/config/item_cfg")
local dynamicData = require("lua/game/data/dynamicData")
local bagOperator = {}
--创建一种物品
--cfgid 配置id
--num 数量
--返回新创建的物品id
function bagOperator:create_one_item(role, cfgid, num, module_id)
    local item = dynamicData.goodsdata:new()
    item.id = newid()
    item.cfgid = cfgid
    item.num = num
    role.goods[item.id] = item

    --更新数据库
    local insertdata = {
        cache_key = 'goods_' .. item.id .. "_" .. role.id,
        opertype = 0,
        fields = {
            { key = '_id', value = tostring(item.id),},
            { key = 'cfgid', value = tostring(item.cfgid),},
            { key = 'num', value = tostring(item.num),},
            { key = 'roleid', value = tostring(role.id),},
        }
    }
    ConnectorMgr:dbConnector():send('insertdata_to_dbvisit_c',insertdata)

    --记录日志
    local change_log = {
        fun = 'bagOperator:create_one_item',
        id = item.id,
        cfgid = cfgid,
        num = num,
    }
    Record_Role_Data_Change(module_id,role.id,Data_type.goods,change_log)

    --通知角色物品变化
    role:send('goodsupdata_client_gate_g',{
        roleid=role.id,
        id=item.id,
        cfgid=item.cfgid,
        num=item.num
    });
    return item.id
end

--获取指定配置id物品在背包中第一个出现的数量,没有返回nil
function bagOperator:get_one_item_num(role,cfgid)
    for k, v in pairs(role.goods) do
        if v.cfgid == cfgid then
            return v.num,k
        end
    end
    return nil,nil
end

--增加一种物品,背包没有此物品则创建物品,否则如果能叠加此物品则叠加,否则创建物品
--role 操作的角色
--cfgid 物品配置id
--num 数量
--module_id 所属模块id
--成功返回物品id,失败返回nil
function bagOperator:add_one_item(role,cfgid,num,module_id)
    --判断能否叠加
    local cfg = item_cfg:getById(cfgid)
    if cfg == nil then
        return nil
    end
    if cfg.Overlap == 0 then
        --不能叠加
        return self:create_one_item(role,cfgid,num,module_id)
    else
        --判断是否超过叠加上限
        local curr_num,item_id = self:get_one_item_num(role,cfgid)
        if curr_num == nil or curr_num + num >= cfg.Overlap then
            --超过叠加上限或者背包中无此物品
            return self:create_one_item(role,cfgid,num,module_id)
        else
            --没有超过叠加上限
            role.goods[item_id].num = role.goods[item_id].num + num

            --更新数据库
            local updata = {
                tag="add_one_item,"..role.id,
                cache_key = "goods_".. item_id .."_" .. role.id,
                opertype = 0,
                fields = {
                    { key = 'num', value = tostring(role.goods[item_id].num),},
                }
            }
            ConnectorMgr:dbConnector():send('updata_to_dbvisit_c',updata)

            --记录日志
            local change_log = {
                fun = 'bagOperator:add_one_item',
                id = item_id,
                cfgid = cfgid,
                change_num = num,
                change_after_num = role.goods[item_id].num
            }
            Record_Role_Data_Change(module_id,role.id,Data_type.goods,change_log)

            --通知角色物品变化
            role:send('goodsupdata_client_gate_g',{
                roleid=role.id,
                id=item_id,
                cfgid=cfgid,
                num=role.goods[item_id].num
            },role.routing);
            return item_id
        end
    end
end

--创建或者增加多种物品,背包没有此物品则创建物品,否则如果能叠加此物品则叠加,否则创建物品
--此方法保证要么全部成功,要么一个都不会成功
--goods = {cfgid,num}
--返回是否成功
function bagOperator:add_more_items(role,goods,module_id)
    local isok = true
    local errdesc
    for k, v in pairs(goods) do
        --验证配置是否存在
        if item_cfg:getById(v.cfgid) == nil then
            isok = false
            errdesc = '配置 cfgid:' .. v.cfgid .. ' 不存在'
            break
        end
    end
    if isok == false then
        return false,errdesc
    end
    for k,v in pairs(goods) do
        self:add_one_item(role,v.cfgid,v.num,module_id)
    end
    return true
end

--减少一种物品的数量,如果物品数量不足则从背包删除此物品
--cfgid 物品配置id
--num 数量
--module_id 所属模块id
--成功返回物品id,失败返回nil
function bagOperator:sub_one_item(role, cfgid, num, module_id)
    local curr_num,item_id = self:get_one_item_num(role,cfgid)
    if curr_num ~= nil then
        if curr_num >= math.abs(num) then
            role.goods[item_id].num = role.goods[item_id].num + num

            --更新数据库
            local updata = {
                tag="sub_one_item,"..role.id,
                cache_key = "goods_".. item_id .."_" .. role.id,
                opertype = 0,
                fields = {
                    { key = 'num', value = tostring(role.goods[item_id].num),},
                }
            }
            ConnectorMgr:dbConnector():send('updata_to_dbvisit_c',updata)

            --记录日志
            local change_log = {
                fun = 'bagOperator:sub_one_item',
                id = item_id,
                cfgid = cfgid,
                change_num = num,
                change_after_num = role.goods[item_id].num
            }
            Record_Role_Data_Change(module_id,role.id,Data_type.goods,change_log)

            --通知角色物品变化
            role:send('goodsupdata_client_gate_g',{
                roleid=role.id,
                id=item_id,
                cfgid=cfgid,
                num=role.goods[item_id].num
            },role.routing);
        else
            self:del_one_item(role, item_id,module_id);
        end
        return item_id
    end
    return nil
end

--减少多种物品的数量,如果物品数量不足则从背包删除此物品
--此方法保证要么全部成功,要么一个都不会成功
--goods = {cfgid,num}
--返回是否成功
function bagOperator:sub_more_item(role,goods,module_id)
    local isok = true
    for k, v in pairs() do
        --验证背包中是否存在此物品
        local curr_num = self:get_one_item_num(role,v.cfgid)
        if curr_num == nil then
            isok = false
            break
        end
    end
    if isok == false then
        return false
    end
    for k,v in pairs(goods) do
        self:sub_one_item(role,v.cfgid,v.num,module_id)
    end
    return true
end

--删除一种物品,删除前不进行验证,请调用前自行验证物品是否存在
function bagOperator:del_one_item(role,goods_id,module_id)
    local cfgid = role.goods[goods_id].cfgid
    role.goods[goods_id] = nil

    --更新数据库
    local deldata = {
        tag="del_one_item,"..role.id,
        cache_key="goods_".. goods_id .."_" .. role.id,
        opertype = 0,
    }
    ConnectorMgr:dbConnector():send('deletedata_to_dbvisit_c',deldata)

    --记录日志
    local change_log = {
        fun = 'bagOperator:del_one_item',
        id = goods_id,
        cfgid = cfgid,
    }
    Record_Role_Data_Change(module_id,role.id,Data_type.goods,change_log)

    --通知角色物品变化
    role:send('goodsupdata_client_gate_g',{
        roleid=role.id,
        id=goods_id,
        cfgid = cfgid,
        num=0,
    },role.routing);
end

--删除多种物品
--此方法保证要么全部成功,要么一个都不会成功
--goods = {id}
--返回是否成功
function bagOperator:del_more_item(role,goods,module_id)
    local isok = true
    for k, v in pairs() do
        --验证背包中是否存在此物品
        if role.goods[v.id] == nil then
            isok = false
            break
        end
    end
    if isok == false then
        return false
    end
    for k,v in pairs(goods) do
        self:del_one_item(role,v.id,module_id)
    end
    return true;
end

--是否存在指定cfgid和num的物品
function bagOperator:exist_one_item(role,cfgid,num)
    local curr_num,item_id = self:get_one_item_num(role,cfgid)
    if curr_num == nil then
        return false
    else
        if curr_num >= num then
            return true,item_id,cfgid
        else
            return false
        end
    end
end

--是否存在指定cfgid和num的多种物品
--goods = {cfgid,num}
function bagOperator:exist_more_item(role,goods)
    local isok = true
    local item_id,cfgid = nil,nil --第一个不满足条件的物品id和配置id
    for k,v in pairs(goods) do
        isok,item_id,cfgid = self:exist_one_item(role,v.cfgid,v.num)
        if isok == false then
            break
        end
    end
    if isok == false then
        return false,item_id,cfgid
    end
    return true
end


return bagOperator