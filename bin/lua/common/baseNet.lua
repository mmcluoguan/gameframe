local pb = require("pb")
pb.loadfile("lua/pb/common.pb")
pb.loadfile("lua/pb/internal.pb")
pb.loadfile("lua/pb/dbvisit.pb")
pb.loadfile("lua/pb/game.pb")
pb.loadfile("lua/pb/login.pb")
pb.loadfile("lua/pb/world.pb")
pb.loadfile("lua/pb/gate.pb")
pb.loadfile("lua/pb/client.pb")

local baseNet = {}

--构造
--client c++指针
function baseNet:new(socket)	
    log("baseNet:new fd:" .. socket:fd())
	local o = {}
	setmetatable(o,self)	
	self.__index = self	
	
	o.cpp_socket = socket;
    --socketID
    o.id = socket:fd()
    if socket:ident() == 1 then
      --连接身份才有name
      o.name = socket:name()
    end
	
	o:init();
	return o
end

--初始化
--sf 自己引用
function baseNet:init(sf)
	if(sf ~= nil) then
		self = sf;
	end
end


--析构
--sf 自己引用
function baseNet:clean(sf)
	if(sf ~= nil) then
		self = sf;
		log("baseNet:clean fd:" .. self.id)
	end
	self.cpp_socket = nil;
end

function baseNet:send_errcode(code,desc,routing)
	local codenum = pb.enum("frmpub.protocc.errnum",string.upper(code))
	return self:send('errcode',{code=codenum,desc=desc,},routing)
end

--发送消息
--msg_name 消息名称
--msg_data 表数据
function baseNet:send(msg, msgdata,routing)
	local msgid = msg
	local msgname = ""
	if type(msgid) == "string" then
		msgname = string.lower(msgid)
		msgid = pb.enum("frmpub.protocc.ClientMsgId",string.upper(msg))
		if msgid == nil then
			msgid = pb.enum("frmpub.protocc.InternalMsgId", string.upper(msg))
			if msgid == nil then
				msgid = pb.enum("frmpub.protocc.BroadcastMsgId", string.upper(msg))
			end
		end
		assert(msg,"msgid=" .. msg .. " 未定义");
	elseif type(msgid) == "number" then
		msgname = string.lower(pb.enum("frmpub.protocc.ClientMsgId", msg))
		if msgname == nil then
			msgname = string.lower(pb.enum("frmpub.protocc.InternalMsgId", msg))
			if msgname == nil then
				msgname = string.lower(pb.enum("frmpub.protocc.BroadcastMsgId", msg))
			end
		end
		assert(msg,"msgid=" .. msg .. " 未定义");
	end
	local bytes = msgdata
	if msgdata ~= nil then
		if type(msgdata) == "table" then
			bytes = pb.encode("frmpub.protocc." .. msgname, msgdata)
		end
	end
    return self.cpp_socket:send_proto(msgid,bytes,routing)
end

--关闭连接
function baseNet:close()
	if self.cpp_socket:ident() == 0 then
		--接收身份,服务器主动断开
		self.cpp_socket:close(true)
	elseif self.cpp_socket:ident() == 1 then
		--连接身份,客户端主动断开
		self.cpp_socket:close(0)
	end
end


--消息默认处理
function baseNet:defaultHandle(msgid,msgname,data,routing)
	log("消息:",msgid,"=",msgname,"没有消息处理函数");
end

--分发消息
function baseNet:handle_message(msgid,data,routing)
	local msgname = pb.enum("frmpub.protocc.ClientMsgId", msgid)
	if msgname == nil then
		msgname = pb.enum("frmpub.protocc.InternalMsgId", msgid)
	end
	if msgname ~= nil then
		msgname = string.lower(msgname)
		if(self[msgname] ~= nil) then
			return self[msgname](self,msgid,data,routing)
		else
			return self:defaultHandle(msgid,msgname,data,routing);
		end
	else
		log("msgid=" .. msgid .. " 未定义")
	end
end

return baseNet;
