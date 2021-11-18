local dynamicData = require("lua/game/data/dynamicData")
local pb = require("pb")
--角色表
local role = {}

function role:new(id,gatefd)
    local o = {}
    setmetatable(o,self)
    self.__index = self

    log("初始化角色 id:",id)
    --角色id
    o.id = id
    --账号id
    o.accountid = nil
    --gate socket标识
    o.gatefd = gatefd
    --client socket标识
    o.clientfd = 0
    --路由信息
    o.routing = StackEnvelope_CPP.new()
    --是否在线,当false时会延迟删除内存数据
    o.online = true
    --最近一次上线时间
    o.online_time = 0
    --最近一次离线时间
    o.unline_time = 0
    --等级
    o.level = 1
    --物品列表
    o.goods = {}
    --邮件列表
    o.emails = {}
    --销毁计时器id
    o.destroy_timerid = 0
    --星级
    o.star = 0
    --游戏币
    o.gold = 0
    --钻石
    o.diamond = 0

    return o
end

function role:clean()
    log("销毁角色 id:",self.id)
    self.unline_time = os.time()
    --更新数据库
    local updata = {
        cache_key = "role_" .. self.id,
        opertype = 0,
        fields = {
            { key = 'level', value = tostring(self.level),},
            { key = 'star', value = tostring(self.star),},
            { key = 'gold', value = tostring(self.gold),},
            { key = 'diamond', value = tostring(self.diamond),},
            { key = 'online_time', value = tostring(self.online_time),},
            { key = 'unline_time', value = tostring(self.unline_time),},
        }
    }
    ConnectorMgr:dbConnector():send('updata_to_dbvisit_c',updata)
end

function role:send(msg, msgdata,routing)
    if routing == nil then
        routing = self.routing
    end
    local gameclient = AcceptMgr:find(self.gatefd)
    if gameclient ~= nil then
       return gameclient:send(msg, msgdata,routing)
    else
       log("网关已断开连接")
    end
end

--拷贝最新路由信息
function role:copyrouting(routing)
    while self.routing:size() ~= 0 do
        self.routing:pop()
    end
    self.routing:copy(routing)
end

--分发消息
function role:handle_message(msgname,data,routing)
    self:copyrouting(routing)
    local fun = self[msgname]
    if fun == nil then
        return false
    else
        return self[msgname](self,data,routing)
    end
end

--重置角色等级
function role:setlevel_client_gate_c(msgdata,routing)
    local msgtable = pb.decode("frmpub.protocc.setlevel_client_gate_c", msgdata)
    self.level = 0
    BaseOperator:change_level(self,msgtable.level,Module_name.test)
    self:send('setlevel_client_gate_s',{ result = 0,level = self.level },routing)
end

--加载邮件列表
function role:loademails_client_gate_c(msgdata,routing)
    if self.is_load_emails == nil then
        --转发消息到db获取邮件列表
        local gameclient = AcceptMgr:find(self.gatefd)
        assert(gameclient)
        local enve = Envelope_CPP.new()
        enve:fd(gameclient.id)
        enve:addr(gameclient.cpp_socket:remote_addr())
        routing:push(enve)
        local emailsdata = {
            tag = "emailsdata," .. self.id,
            condition = "email_*_" .. self.id,
            sort = "_id desc",
            limit = 100,
            opertype = 0,
            fields = {
                {key = '_id', value = '',},
                {key = 'is_read', value = '0',},
                {key = 'is_receive', value = '0',},
            },
        }
        ConnectorMgr:dbConnector():send('loaddata_more_from_dbvisit_c',emailsdata,routing)
       
    else
        log("在本地内存中获取角色邮件数据 len:",get_tablekey_size(self.emails))        
        local msgname,msgtable = self:emails_data()
        self:send(msgname,msgtable,routing)
    end
end

--邮件列表proto数据
function role:emails_data()
    self.is_load_emails = true
    EmailOperator:collect_unline_email(self)
    local emailsdata = {}
    local i = 1
    for k,v in pairs(self.emails) do
        table.insert(emailsdata,i,v)
        i = i + 1
    end
    return 'loademails_client_gate_s',{ emails = emailsdata}
end

--查看邮件信息
function role:lookemail_client_gate_c(msgdata,routing)
    local msgtable = pb.decode("frmpub.protocc.lookemail_client_gate_c", msgdata)
    local msgsdata ={
        result = 0,
    }
    local email = self.emails[msgtable.emailid]
    if email ~= nil then
        email:set_readed(self)
        email = EmailSystem.emails[email.id]
        msgsdata.id = email.id        
        msgsdata.title = email.title
        msgsdata.info = email.info
        msgsdata.type = email.type
        msgsdata.time = email.time
        msgsdata.annex = email.annex
    else
        msgsdata.result = 1 --邮件不存在
    end
    self:send('lookemail_client_gate_s',msgsdata,routing)
end

--领取邮件附件
function role:getannex_client_gate_c(msgdata,routing)
    local msgtable = pb.decode("frmpub.protocc.getannex_client_gate_c", msgdata)
    local msgsdata ={
        result = 0,
    }
    local email = self.emails[msgtable.emailid]
    if email ~= nil then
        if email.is_receive == true then
            msgsdata.result = 3 --已领取
        else
            local mateEmail = EmailSystem.emails[email.id]
            if mateEmail.annex == nil then
                msgsdata.result = 2 --没有附件
            else
                if mateEmail.annex.gold ~= nil then
                    BaseOperator:change_gold(self,mateEmail.annex.gold,Module_name.email)
                end
                if mateEmail.annex.diamond ~= nil then
                    BaseOperator:change_diamond(self,mateEmail.annex.diamond,Module_name.email)
                end
                if mateEmail.annex.goods ~= nil then
                    local f,errdesc = BagOperator:add_more_items(self,mateEmail.annex.goods,Module_name.email)
                    assert(f==true,errdesc)
                end                
                email:set_received(self)
            end
        end
    else
        msgsdata.result = 1 --邮件不存在
    end
    self:send('getannex_client_gate_s',msgsdata,routing)
end

return role