--基础数据操作
local baseOperator = {}

--通知角色基础数据变化
function baseOperator:notice_base_change(role)    
    role:send('baseupdata_client_gate_g',{
        roleid=role.id,
        level=role.level,
        gold=role.gold,
        diamond=role.diamond
    },role.routing);
end

--角色游戏币改变
function baseOperator:change_gold(role,gold,module_id)
    role.gold = role.gold + gold

    --更新数据库
    local updata = {
        cache_key = "role_" .. role.id,
        opertype = 0,
        fields = {
            { key = 'gold', value = tostring(role.gold),},
        }
    }
    ConnectorMgr:dbConnector():send('updata_to_dbvisit_c',updata)

    --记录日志
    local change_log = {
        fun = 'baseOperator:change_gold',
        change_gold = gold,
        change_after_gold = role.gold,
    }
    Record_Role_Data_Change(module_id,role.id,Data_type.base,change_log)

    self:notice_base_change(role)
end

--角色钻石改变
function baseOperator:change_diamond(role,diamond,module_id)
    role.diamond = role.diamond + diamond

    --更新数据库
    local updata = {
        cache_key = "role_" .. role.id,
        opertype = 0,
        fields = {
            { key = 'diamond', value = tostring(role.diamond),},
        }
    }
    ConnectorMgr:dbConnector():send('updata_to_dbvisit_c',updata)

    --记录日志
    local change_log = {
        fun = 'baseOperator:change_diamond',
        change_diamond = diamond,
        change_after_diamond = role.diamond,
    }
    Record_Role_Data_Change(module_id,role.id,Data_type.base,change_log)

    self:notice_base_change(role)
end

--角色等级改变
function baseOperator:change_level(role,level,module_id)
    role.level = role.level + level

    --更新数据库
    local updata = {
        cache_key = "role_" .. role.id,
        opertype = 0,
        fields = {
            { key = 'level', value = tostring(role.level),},
        }
    }
    ConnectorMgr:dbConnector():send('updata_to_dbvisit_c',updata)

    --记录日志
    local change_log = {
        fun = 'baseOperator:change_level',
        change_level = level,
        change_after_level = role.level,
    }
    Record_Role_Data_Change(module_id,role.id,Data_type.base,change_log)

    self:notice_base_change(role)
end

return baseOperator