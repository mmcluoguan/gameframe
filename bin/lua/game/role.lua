local global_cfg = require("lua/config/global_cfg")
local pb = require("pb")
--角色表
local role = {}

function role:new(roleid,accountid)
    local o = {}
    setmetatable(o,self)
    self.__index = self

    log("初始化角色 id:",roleid)
    --角色id
    o.id = roleid
    --账号id
    o.accountid = accountid
    --路由信息
    o.routing = StackEnvelope_CPP.new()
    --销毁计时器id
    o.destroy_timerid = 0

    --物品列表
    o.goods = {}
    --邮件列表
    o.emails = {}

    return o
end

--初始化需要持久化的简单数据
function role:init_fields()
    local fields = {} 
    --角色id
    table.insert(fields,{ key ='_id',value = tostring(self.id)})
    --账号id
    table.insert(fields,{ key ='accountid',value = tostring(self.accountid)})
    --是否在线,当false时会延迟删除内存数据
    self.online = true
    table.insert(fields,{ key ='online',value = tostring(self.online)})
    --最近一次上线时间
    self.online_time = 0
    table.insert(fields,{ key ='online_time',value = tostring(self.online_time)})
    --最近一次离线时间
    self.unline_time = 0
    table.insert(fields,{ key ='unline_time',value = tostring(self.unline_time)})
    --等级
    self.level = random(1,100)
    table.insert(fields,{ key ='level',value = tostring(self.level)})
    --星级
    self.star = 0
    table.insert(fields,{ key ='star',value = tostring(self.star)})
    --游戏币
    self.gold = 0
    table.insert(fields,{ key ='gold',value = tostring(self.gold)})
    --钻石
    self.diamond = 0
    table.insert(fields,{ key ='diamond',value = tostring(self.diamond)})
    --剩余抽奖次数
    self.lottery = global_cfg:getByKey("每日抽奖次数").Value
    table.insert(fields,{ key ='lottery',value = tostring(self.lottery)})

    return fields
end

--返回客户端需要的角色数据
function role:client_roledata()
    return {
        roleid = self.id,
        aid = self.accountid,
        level = self.level,
        gold = self.gold,
        diamond = self.diamond,
        lottery = self.lottery,
    } 
end

--角色数据加载完成
function role:loaddata_complete(gatefd,clientfd,dbdata)    
    --gate socket标识
    self.gatefd = gatefd
    --client socket标识
    self.clientfd = clientfd

    if dbdata ~= nil then
        --装载来自db的数据
        for i = 1, #dbdata do
            local key = dbdata[i].key
            if key == '_id' then
                self.id = tonumber(dbdata[i].value)
            else
                local t = self[key]
                assert(t,'角色没有字段 '.. key)
                if type(t) == "string" then
                    self[key] = tostring(dbdata[i].value)
                elseif type(t) == "number" then
                    self[key] = tonumber(dbdata[i].value)
                elseif type(t) == "boolean" then
                    self[key] = StrtoBool(dbdata[i].value)
                end
            end
        end
    end

    --是否在线,当false时会延迟删除内存数据
    self.online = true
    --最近一次上线时间
    self.online_time = os.time()

    if self.online_time - self.unline_time >= 24 * 60 * 60 then
        self:resetdata()
    end

    self:save('online','online_time')
end

--保存数据
--eg 'gold','diamond'
function role:save(...)
    assert(self.id,'没有设置角色id')
    --更新数据库
    local updata = {
        cache_key = "role_" .. self.id,
        opertype = 0,
    }
    updata.fields = {}
    for key, _ in pairs(...) do
        local t = {}
        t.key = key
        assert(role[key],'角色没有字段 '.. key)
        t.value = tostring(role[key])
        table.insert(updata.fields,t)
    end
    ConnectorMgr:dbConnector():send('updata_to_dbvisit_c',updata)
end

function role:clean()
    log("销毁角色 id:",self.id)
    self.unline_time = os.time()
    self.online = false

    self:save('online','unline_time')
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

--重置数据
function role:resetdata()
    self.lottery = global_cfg:getByKey("每日抽奖次数").Value
    self:save('lottery')
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
        log("在本地内存中获取角色邮件数据 len:",Get_Tablekey_Size(self.emails))        
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