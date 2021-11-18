--物品表
local goodsdata = {}

function goodsdata:new()
    local o = {}
    setmetatable(o,self)
    self.__index = self

    o.id = nil --唯一id
    o.cfgid = nil --配置id
    o.num = nil   --数量

    return o;
end

--邮件表
local emaildata = {}

function emaildata:new()
    local o = {}
    setmetatable(o,self)
    self.__index = self

    o.id = nil    --唯一id,关联emailMatedata
    o.is_read = false --是否已读
    o.is_receive = false --是否已领取附件

    return o;
end

--设置已读
function emaildata:set_readed(role)
    self.is_read = true

    --更新数据库
    local updata = {
        cache_key = "email_" .. self.id .. "_" .. role.id,
        opertype = 0,
        fields = {
            { key = 'is_read', value = tostring(self.is_read),},
        }
    }
    ConnectorMgr:dbConnector():send('updata_to_dbvisit_c',updata)
end

--设置已领取附件
function emaildata:set_received(role)
    self.is_receive = true

    --更新数据库
    local updata = {
        cache_key = "email_" .. self.id .. "_" .. role.id,
        opertype = 0,
        fields = {
            { key = 'is_receive', value = tostring(self.is_receive),},
        }
    }
    ConnectorMgr:dbConnector():send('updata_to_dbvisit_c',updata)
end

local dynamicData = {
    goodsdata = goodsdata,
    emaildata = emaildata,
}

return dynamicData