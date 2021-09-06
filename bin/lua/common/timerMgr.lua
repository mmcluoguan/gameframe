local timerMgr = {}

--计时器回调
--timer_id 计时器id
function onTimerOut(timer_id)
    --print('计时器回调',timer_id)
    timerMgr:execute(timer_id)
end

--绑定计时器
--millisecond 毫秒
--fun_ptr 函数
--para_ptr 调用函数参数列表
--repeated 是否重复执行 默认true 值为false不需要unbind
function timerMgr:bind(millisecond,fun_ptr,para_ptr,repeated)
    if repeated == nil then
        repeated = true
    end
    local tb = {
        millisecond = millisecond,
        fun = fun_ptr,
        para = para_ptr,
        repeated = repeated,
    }
    local timer_id = schedule_timer(millisecond,repeated)
    --print("绑定计时器",timer_id)
    self[timer_id] = tb
    return timer_id
end

--取消计时器
function timerMgr:unbind(timer_id)
    --print("取消计时器",timer_id)
    cancel_timer(timer_id)
    self[timer_id] = nil
end

function timerMgr:execute(timer_id)
    local tb = self[timer_id];
    if tb.para == nil then
        tb.fun();
    elseif #tb.para == 0 then
        tb.fun();
    elseif #tb.para == 1 then
        tb.fun(tb.para[1]);
    elseif #tb.para == 2 then
        tb.fun(tb.para[1],tb.para[2]);
    elseif #tb.para == 3 then
        tb.fun(tb.para[1],tb.para[2],tb.para[3]);
    elseif #tb.para == 4 then
        tb.fun(tb.para[1],tb.para[2],tb.para[3],tb.para[4]);
    elseif #tb.para == 5 then
        tb.fun(tb.para[1],tb.para[2],tb.para[3],tb.para[4],tb.para[5]);
    elseif #tb.para == 6 then
        tb.fun(tb.para[1],tb.para[2],tb.para[3],tb.para[4],tb.para[5],tb.para[6]);
    elseif #tb.para == 7 then
        tb.fun(tb.para[1],tb.para[2],tb.para[3],tb.para[4],tb.para[5],tb.para[6],tb.para[7]);
    end
    if tb.repeated == false then
        self[timer_id] = nil
    end
end

return timerMgr;
