/**
 * @file    tasks_order.cpp
 * <pre>
 * Copyright (c) 2019, Gaaagaa All rights reserved.
 * 
 * 文件名称：tasks_order.cpp
 * 创建日期：2019年10月16日
 * 文件标识：
 * 文件摘要：测试 线程池 的 “使某一批任务对象在线程池中可按提交次序而顺序执行” 功能。
 * 
 * 当前版本：1.0.0.0
 * 作    者：
 * 完成日期：2019年10月16日
 * 版本摘要：
 * 
 * 历史版本：
 * 原作者  ：
 * 完成日期：
 * 版本摘要：
 * </pre>
 */

#include "xthreadpool.h"
#include <chrono>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

////////////////////////////////////////////////////////////////////////////////

class x_order_task_t;

/**
 * @class objectT
 * @brief 产生 x_order_task_t 任务对象序列的测试类。
 */
class objectT
{
    // constructor/destructor
public:
    objectT(const std::string & xstr_name)
        : m_xrunning_flag(false)
        , m_seqno_taskid(0)
        , m_xstr_name(xstr_name)
    {

    }

    // public interfaces
public:
    inline void set_running(bool xrunning_flag)
    {
        m_xrunning_flag = xrunning_flag;
    }

    inline bool is_running(void) const
    {
        return m_xrunning_flag;
    }

    inline const std::string & name(void) const
    {
        return m_xstr_name;
    }

    x_order_task_t * new_task(void);

    // data members
private:
    bool         m_xrunning_flag;
    int          m_seqno_taskid;
    std::string  m_xstr_name;
};

/**
 * @class x_order_task_t
 * @brief 保证顺序执行的测试任务对象。
 */
class x_order_task_t : public x_threadpool_t::x_task_t
{
    // constructor/destructor
public:
    x_order_task_t(objectT * obj_owner, int taskid)
        : m_obj_owner(obj_owner)
        , m_taskid(taskid)
    {

    }

    virtual ~x_order_task_t(void) { }

    // overrides
public:
    virtual void run(x_running_checker_t * xchecker_ptr) override
    {
        printf("%s:%02d\n", m_obj_owner->name().c_str(), m_taskid);
    }

    virtual bool is_suspend(void) const override
    {
        return m_obj_owner->is_running();
    }

    virtual void set_running_flag(bool xrunning_flag) override
    {
        m_obj_owner->set_running(xrunning_flag);
    }

    // data members
private:
    objectT * m_obj_owner;
    int       m_taskid;
};

x_order_task_t * objectT::new_task(void)
{
    return (new x_order_task_t(this, m_seqno_taskid++));
}

//====================================================================

int main(int argc, char * argv[])
{
    // 线程池对象
    x_threadpool_t xht_pool;

    // 启动线程池
    if (!xht_pool.startup(0))
    {
        printf("startup return false!\n");
        return -1;
    }

    //======================================
    // 便于测试而增加的代码
    // 提交任务对象 : 使线程池的所有工作线程处于暂停状态

    bool is_pause_pool = true;
    for (int iter = 0, nthds = (int)xht_pool.size(); iter < nthds + 1; ++iter)
    {
        xht_pool.submit_task_ex(
            [&is_pause_pool]() -> void
            {
                while (is_pause_pool)
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                }
            });
    }

    //======================================
    // 提交任务对象 : 按提交顺序执行任务对象

    srand(time(NULL));

    objectT objA("objA");
    objectT objB("objB");
    for (int iter = 0; iter < 100; ++iter)
    {
        if (0 == (rand() % 2))
        {
            xht_pool.submit_task(objA.new_task());
        }
        else
        {
            xht_pool.submit_task(objB.new_task());
        }
    }

    // 使线程池开始工作（便于测试而增加的代码）
    is_pause_pool = false;

    //======================================

    // 等待所有任务执行完成
    while (xht_pool.task_count() > 0)
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

    // 关闭线程池
    xht_pool.shutdown();

    return 0;
}

