#include "event.h"

#define GET_MS_TICK                     HAL_GetTick

#define EVENT_QUEUE_SIZE                16
#define EVENT_DISPATCH_MAX_PER_LOOP     8

/**
 * @brief 事件消息类型
 */
enum class event_msg_type
{
    timer = 0,
    async
};

/**
 * @brief 事件队列节点
 */
struct event_msg
{
    event_msg_type type;
    event *target;
    uint32_t id;
    uint32_t data;
};

static event *head = nullptr, *tail = nullptr;
static void (*app_handler)(uint32_t, uint32_t) = nullptr;

static event_msg queue[EVENT_QUEUE_SIZE];
static volatile uint8_t q_read = 0, q_write = 0;
static volatile uint32_t q_count = 0, q_drops = 0;

/**
 * @brief 关闭中断并保存原中断状态
 */
static uint32_t lock_irq()
{
    uint32_t primask = __get_PRIMASK();
    __disable_irq();
    return primask;
}

/**
 * @brief 恢复进入临界区前的中断状态
 */
static void unlock_irq(uint32_t primask)
{
    if((primask & 1) == 0)
    {
        __enable_irq();
    }
}

/**
 * @brief 获取环形队列的下一个下标
 */
static uint8_t next_index(uint8_t index)
{
    index++;
    if(index >= EVENT_QUEUE_SIZE)
    {
        index = 0;
    }
    return index;
}

/**
 * @brief 压入一个事件消息
 */
static bool push(const event_msg &msg)
{
    uint32_t primask = lock_irq();

    if(q_count >= EVENT_QUEUE_SIZE)
    {
        q_drops++;
        unlock_irq(primask);
        return false;
    }

    queue[q_write] = msg;
    q_write = next_index(q_write);
    q_count++;

    unlock_irq(primask);
    return true;
}

/**
 * @brief 弹出一个事件消息
 */
static bool pop(event_msg &msg)
{
    uint32_t primask = lock_irq();

    if(q_count == 0)
    {
        unlock_irq(primask);
        return false;
    }

    msg = queue[q_read];
    q_read = next_index(q_read);
    q_count--;

    unlock_irq(primask);
    return true;
}

/**
 * @brief 构造周期事件
 */
event::event(uint32_t period, void (*event_proc)(uint32_t))
    : period(period),
      proc(event_proc),
      proc_void(nullptr){}

/**
 * @brief 构造无参数周期事件
 */
event::event(uint32_t period, void (*event_proc_void)())
    : period(period),
      proc(nullptr),
      proc_void(event_proc_void){}

/**
 * @brief 启动周期事件
 */
void event::start()
{
    if(running){return;}

    last_time = GET_MS_TICK();
    queued = false;
    running = true;

    if(listed){return;}

    next = nullptr;
    listed = true;

    if(head)
    {
        tail->next = this;
        tail = this;
    }
    else
    {
        head = this;
        tail = this;
    }
}

/**
 * @brief 停止周期事件
 */
void event::stop()
{
    running = false;
    queued = false;
}

/**
 * @brief 事件调度主循环
 */
void event::loop()
{
    poll(GET_MS_TICK());
    dispatch(EVENT_DISPATCH_MAX_PER_LOOP);
}

/**
 * @brief 投递异步事件
 */
bool event::post(uint32_t id, uint32_t data)
{
    event_msg msg;

    msg.type = event_msg_type::async;
    msg.target = nullptr;
    msg.id = id;
    msg.data = data;

    return push(msg);
}

/**
 * @brief 注册异步事件处理函数
 */
void event::register_handler(void (*handler)(uint32_t, uint32_t))
{
    app_handler = handler;
}

/**
 * @brief 获取事件丢弃计数
 */
uint32_t event::drops()
{
    return q_drops;
}

/**
 * @brief 执行周期事件回调
 */
void event::run(uint32_t tick)
{
    if(proc)
    {
        proc(tick);
    }
    else if(proc_void)
    {
        proc_void();
    }
}

/**
 * @brief 检查周期事件是否到期
 */
void event::poll(uint32_t now)
{
    event *p = head;
    while(p)
    {
        if(p->running && !p->queued && p->period > 0 && now - p->last_time >= p->period)
        {
            p->last_time += p->period;
            p->queued = push_timer(p);
        }
        p = p->next;
    }
}

/**
 * @brief 分发事件队列中的消息
 */
void event::dispatch(uint8_t max_events)
{
    event_msg msg;
    while(max_events-- && pop(msg))
    {
        if(msg.type == event_msg_type::timer)
        {
            msg.target->queued = false;
            if(msg.target->running)
            {
                msg.target->run(msg.target->period);
            }
        }
        else if(app_handler)
        {
            app_handler(msg.id, msg.data);
        }
    }
}

/**
 * @brief 将周期事件压入事件队列
 */
bool event::push_timer(event *target)
{
    event_msg msg;
    msg.type = event_msg_type::timer;
    msg.target = target;
    msg.id = 0;
    msg.data = 0;

    return push(msg);
}
