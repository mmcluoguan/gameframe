local pb = require "pb"
local protoc = require "lua/common/protoc"

-- 直接载入schema (这么写只是方便, 生产环境推荐使用 protoc.new() 接口)
--assert(protoc:load [[
--   message Phone {
--      optional string name        = 1;
--      optional int64  phonenumber = 2;
--   }
--   message Person {
--      optional string name     = 1;
--      optional int32  age      = 2;
--      optional string address  = 3;
--      repeated Phone  contacts = 4;
--   } ]])

assert(pb.loadfile "lua/pb/person.pb")

-- lua 表数据
local data = {
   name = "ilseaa",
   age  = 181,
   contacts = {
      { name = "alice", phonenumber = 12312341234 },
      { name = "bob",   phonenumber = 45645674567 }
   }
}

-- 将Lua表编码为二进制数据
local bytes = assert(pb.encode("Person", data))
print(pb.tohex(bytes))

-- 再解码回Lua表
local data2 = assert(pb.decode("Person", bytes))
print(require "lua/common/serpent".block(data2))

print("aaaaaaaaaa",data2.name,data2.age,data2.contacts)

