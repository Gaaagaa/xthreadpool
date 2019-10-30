# C++11 —— 使用 thread 实现线程池

## 1. 引言

在新的 C++11 标准中，引入并发编程的一些基础组件：**线程（thread）**、**互斥锁（mutex）**、**条件变量（condition_variable）** 等，凭借这些，就足够我设计一个平台无关的 **线程池** 组件了。下面就详细介绍一下这个线程池组件。

## 2. 结构设计图

![x_threadpool_t](https://raw.githubusercontent.com/Gaaagaa/xthreadpool/master/xthreadpool.jpg)

需要特别说明的是，这个线程池组件，在增加了“存在关联性的任务对象顺序执行”的功能后，原本的任务队列就分成了两级任务队列，目的是为了降低 **“任务提交”** 与 **“任务提取”** 之间（属于一种生产/消费的关系）的锁竞争。

## 3. 源码说明

源码有点多，这里就不贴出来了，直接给下载地址：[https://github.com/Gaaagaa/xthreadpool](https://github.com/Gaaagaa/xthreadpool) 。主要的线程池类 **x_threadpool_t** 在 **xthreadpool.h** 中已完整实现，在实际项目应用中，只需要 **xthreadpool.h** 这一个文件就足够了。

测试程序的编译命令：
> 1. MSVC++2017：cl /EHsc main.cpp
> 2. gcc ：g++ -Wall -std=c++11 -lpthread -o main main.cpp

技术特点：
> 1. 使用 C++11 的 thread 实现，可跨平台，亲测的编译器有 MSVC++2017、gcc 4.8.5、gcc 8.2.0；
> 2. 支持传统的面向对象编程的任务对象类接口：继承抽象任务对象接口类，实现多态（可结合对象池的模式进行资源复用）；
> 3. 支持泛型接口的任务对象，如：C 函数接口、lambda 表达式、仿函数对象、类对象的成员函数调用；
> 4. 支持动态调整工作线程的数量：通过 resize() 接口实现，该功能当前的实现方式还不够好，仍有优化的方式；
> 5. 支持运行时的线程状态检测（判断当前工作线程是否需要退出）；
> 6. 存在关联性的任务对象可顺序执行（这一特点只针对一些特别的应用场景，后续示例中会展示）。

## 4. 使用说明与示例代码

#### 4.1 启动与关闭

```
#include "xthreadpool.h"
#include <chrono>
#include <stdio.h>

int main(int argc, char * argv[])
{
    // 工作线程数量
    // 若为 0，将取 hardware_concurrency() 返回值的 2倍 + 1
    int nthreads = 4;

    // 线程池对象
    x_threadpool_t xht_pool;

    // 启动线程池
    if (!xht_pool.startup(nthreads))
    {
        printf("startup return false!\n");
        return -1;
    }

    //======================================
    // 提交任务对象
    // ......

    //======================================

    // 等待所有任务执行完成
    while (xht_pool.task_count() > 0)
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

    // 关闭线程池
    xht_pool.shutdown();

    return 0;
}
```

#### 4.2 提交“传统 C 回调函数”的任务对象

```
#include "xthreadpool.h"
#include <chrono>
#include <stdio.h>

void func_task(int task_id, int task_iter)
{
    int count = 1;
    do
    {
        printf("func_task[%d, %d] => count: %d\n", task_id, task_iter, count);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    } while (count++ < 10);
}

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
    // 提交任务对象 : C 函数接口的任务

    for (int iter = 0; iter < 100; iter += 10)
    {
        xht_pool.submit_task_ex(func_task, iter, iter * iter);
    }

    //======================================

    // 等待所有任务执行完成
    while (xht_pool.task_count() > 0)
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

    // 关闭线程池
    xht_pool.shutdown();

    return 0;
}
```

#### 4.3 提交“仿函数对象”的任务对象

```
#include "xthreadpool.h"
#include <chrono>
#include <stdio.h>

/**
 * @struct functor_task_A
 * @brief  仿函数模式的任务对象类。
 */
struct functor_task_A
{
    // constructor/destructor
public:
    functor_task_A(int xtask_id = 0) : m_xtask_id(xtask_id) { }

public:
    void operator()() const
    {
        int count = 1;
        do
        {
            printf("functor_task_A[%d] => count: %d\n", m_xtask_id, count);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        } while (count++ < 10);
    }

    // data members
private:
    int m_xtask_id;
};

/**
 * @struct functor_task_B
 * @brief  仿函数模式的任务对象类。
 */
struct functor_task_B
{
    // constructor/destructor
public:
    functor_task_B(int xtask_id = 0) : m_xtask_id(xtask_id) { }

public:
    void operator()(int flag) const
    {
        int count = 1;
        do
        {
            printf("functor_task_B[%d, %d] => count: %d\n", m_xtask_id, flag, count);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        } while (count++ < 10);
    }

    // data members
private:
    int m_xtask_id;
};

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
    // 提交任务对象 : 仿函数对象

    for (int iter = 0; iter < 100; iter += 10)
    {
        xht_pool.submit_task_ex((functor_task_A(iter)));
    }

    for (int iter = 0; iter < 100; iter += 10)
    {
        xht_pool.submit_task_ex((functor_task_B(iter)), iter * iter / 2);
    }

    //======================================

    // 等待所有任务执行完成
    while (xht_pool.task_count() > 0)
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

    // 关闭线程池
    xht_pool.shutdown();

    return 0;
}
```

#### 4.4 提交“调用类对象的成员函数”的任务对象

```
#include "xthreadpool.h"
#include <chrono>
#include <stdio.h>

/**
 * @class memfunc_task
 * @brief 调用成员函数的任务对象类。
 */
class memfunc_task
{
    // constructor/destructor
public:
    memfunc_task(int xtask_id = 0) : m_xtask_id(xtask_id) {  }

    // overrides
public:
    /**********************************************************/
    /**
     * @brief 任务对象执行流程的操作接口。
     */
    void memfunc(int task_iter)
    {
        int count = 1;
        do
        {
            printf("memfunc_task[%d, %d] => count: %d\n", m_xtask_id, task_iter, count);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        } while (count++ < 10);
    }

    // data members
private:
    int m_xtask_id;
};

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
    // 提交任务对象

    // 注意，这个栈区对象的生命期需要在线程池关闭前存活，
    // 至少要保证所提交的任务都执行完成时，才可结束该对象的生命期
    memfunc_task mftask(0);

    for (int iter = 0; iter < 100; iter += 10)
    {
        xht_pool.submit_task_ex(&memfunc_task::memfunc, &mftask, iter);
    }

    //======================================

    // 等待所有任务执行完成
    while (xht_pool.task_count() > 0)
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

    // 关闭线程池
    xht_pool.shutdown();

    return 0;
}
```

#### 4.5 提交“重载后的任务对象类”的任务对象

```
#include "xthreadpool.h"
#include <chrono>
#include <stdio.h>

/**
 * @class user_task
 * @brief 用户自定义的任务对象类。
 */
class user_task : public x_task_t
{
    // constructor/destructor
public:
    user_task(int xtask_id = 0) : m_xtask_id(xtask_id) {  }

    // overrides
public:
    /**********************************************************/
    /**
     * @brief 任务对象执行流程的操作接口。
     */
    virtual void run(x_running_checker_t * xchecker_ptr) override
    {
        int count = 1;
        do
        {
            printf("[%d]user_task[%d] => count: %d\n", (int)xchecker_ptr->thread_index(), m_xtask_id, count);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        } while ((count++ < 10) && xchecker_ptr->is_enable_running());
    }

    // data members
private:
    int m_xtask_id;
};

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
    // 提交任务对象 : 重载的任务对象

    for (int iter = 0; iter < 100; iter += 10)
    {
        xht_pool.submit_task((x_task_ptr_t)(new user_task(iter)));
    }

    //======================================

    // 等待所有任务执行完成
    while (xht_pool.task_count() > 0)
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

    // 关闭线程池
    xht_pool.shutdown();

    return 0;
}
```

#### 4.6 提交“lambda表达式”的任务对象

```
#include "xthreadpool.h"
#include <chrono>
#include <stdio.h>

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
    // 提交任务对象 : lambda 表达式

    for (int iter = 0; iter < 100; iter += 10)
    {
        xht_pool.submit_task_ex(
            [iter]() -> void
            {
                int task_id = iter;
                for (int jter = 0; jter < 10; ++jter)
                {
                    printf("lambda A task id : %d -> %d\n", task_id, task_id + jter);
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
            });
    }

    for (int iter = 0; iter < 100; iter += 10)
    {
        xht_pool.submit_task_ex(
            [iter](const std::string & str_name) -> void
            {
                int task_id = iter;
                for (int jter = 0; jter < 10; ++jter)
                {
                    printf("lambda B task id : %d -> %d name : %s\n", task_id, task_id + jter, str_name.c_str());
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
            }, std::string("Lambda B"));
    }

    //======================================

    // 等待所有任务执行完成
    while (xht_pool.task_count() > 0)
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

    // 关闭线程池
    xht_pool.shutdown();

    return 0;
}
```

#### 4.7 任务对象执行过程中检测工作线程是否可继续运行

x_threadpool_t 内部提供了一个类 **x_running_checker_t** 可检测当前工作线程是否可继续执行下去（或者说，是否要立即终止执行的任务流程）。提供此功能，主要目的是针对那些耗时长的任务对象（处于运行状态），在线程池关闭或者需要动态调整（减少）工作线程数量时，能够优雅地终止任务流程。参看如下代码是如何使用的：

```
#include "xthreadpool.h"
#include <chrono>
#include <stdio.h>

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
    // 提交任务对象 : lambda 表达式（检测工作线程的退出标识）

    for (int iter = 0; iter < 100; iter += 10)
    {
        xht_pool.submit_task_ex(
            [iter](x_running_checker_t * xchecker_ptr) -> void
            {
                int task_id = iter;
                for (int jter = 0; (jter < 10) && xchecker_ptr->is_enable_running(); ++jter)
                {
                    printf("rchecker[%d] lambda A task id : %d -> %d\n", (int)xchecker_ptr->thread_index(), task_id, task_id + jter);
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
            },
            x_running_checker_t::xholder());
    }

    //======================================

    // 等待所有任务执行完成
    while (xht_pool.task_count() > 0)
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

    // 关闭线程池
    xht_pool.shutdown();

    return 0;
}
```

#### 4.8 使某一批任务对象在线程池中可按提交次序而顺序执行

单看 “按提交次序而顺序执行” 这句话，可能不好理解，先看后面提到的这样应用场景。

A 对象产生（需要执行）的任务对象序列为[ A1, A2, ..., Am ]，并按此顺序依次提交到线程池中；与此同时，B 对象产生（需要执行）的任务对象序列为[ B1, B2, ..., Bn ]，也按此顺序依次提交到线程池中。而不管是 A 对象还是 B 对象，都期望按所提交顺序执行任务对象。这种情况下，在线程池中的任务队列可能有如下图所示的排列：

![order](https://raw.githubusercontent.com/Gaaagaa/xthreadpool/master/tasks_order.jpg)

从本质上看，我们只需要保证 A（或 B） 对象产生的任务在同一时刻只能有一个工作线程执行，这就能保证其顺序性。
面对这种应用场景，我在设计任务对象的抽象基类 **x_task_t** 时，就为此做好了相关的扩展接口。先看下 x_task_t 代码中的两个接口：
```
struct x_task_t
{
    ......

    // extensible interfaces
public:
    ......

    /**********************************************************/
    /**
     * @brief 判断 任务对象 是否挂起。
     * @note  若任务对象处于挂起状态，工作线程提取任务时，则跳过该对象。
     */
    virtual bool is_suspend(void) const { return false; }

    /**********************************************************/
    /**
     * @brief 设置任务对象的运行标识。
     * 
     * @note
     * <pre>
     *   工作线程在提取到任务对象后，则立即调用 set_running_flag(true) 操作；
     *   执行 run() 操作返回后，又调用 set_running_flag(false) 操作。
     * </pre>
     */
    virtual void set_running_flag(bool xrunning_flag) { }

    ......
};
```

再看下 x_threadpool_t 工作线程 “提取任务对象” 以及 “执行任务对象” 的部分实现流程（注意任务对象 is_suspend() 和 set_running_flag() 两个接口被调用的地方）：

```
class x_threadpool_t
{
    ......

    x_task_ptr_t get_task(void)
    {
        ......

        for (std::list< x_task_ptr_t >::iterator itlst = m_lst_run_tasks.begin();
             (itlst != m_lst_run_tasks.end()) && is_enable_get_task();
             ++itlst)
        {
            if ((nullptr == *itlst) || !(*itlst)->is_suspend())
            {
                xtask_ptr = *itlst;
                m_lst_run_tasks.erase(itlst);
                m_xst_lst_tasks.fetch_sub(1);
                break;
            }
        }

        if (nullptr != xtask_ptr)
        {
            xtask_ptr->set_running_flag(true);
        }

        return xtask_ptr;
    }

    ......

    void thread_run(size_t xthread_index)
    {
        ......

            xtask_ptr = get_task();
            if (nullptr == xtask_ptr)
            {
                if (get_lst_task_size() > 0)
                    thread_yield(xcounter);
                continue;
            }

            if (xht_checker.is_enable_running())
            {
                xtask_ptr->run(&xht_checker);
            }

            // 执行完任务对象后，将任务对象转换为 非挂起状态，
            // 加锁进行操作，是为了与 get_task() 内的操作保持队列的同步
            {
                // 标识当前不可提取待执行的任务对象，迫使 get_task() 内部迅速解锁
                m_xst_get_task.fetch_add(1);

                m_lock_run_task.lock();
                xtask_ptr->set_running_flag(false);
                m_lock_run_task.unlock();

                m_xst_get_task.fetch_sub(1);
            }

        ......
    }

    ......
};
```

我们可通过重载 **x_task_t** 所提供的两个扩展接口：is_suspend() 和 set_running_flag() 进行状态切换（即在调用 set_running_flag() 时让相关任务切换 is_suspend() 挂起/非挂起 两种状态），就可实现顺序执行。

示例代码如下所示：

```
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
    if (!xht_pool.startup(0, true))
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
```

看一下测试的输出结果：

```
objA:00
objB:00
objB:01
objB:02
objB:03
objB:04
objA:01
objA:02
objA:03
objB:05
objA:04
objB:06
objB:07
objB:08
objB:09
objA:05
objA:06
objB:10
objB:11
objB:12
objA:07
objA:08
objA:09
objA:10
objB:13
objA:11
objA:12
objB:14
objB:15
objB:16
objB:17
objB:18
objB:19
objA:13
objA:14
objA:15
objA:16
objA:17
objA:18
objA:19
objB:20
objA:20
objA:21
objA:22
objB:21
objB:22
objB:23
objA:23
objB:24
objA:24
objB:25
objB:26
objB:27
objB:28
objB:29
objA:25
objB:30
objA:26
objA:27
objB:31
objB:32
objB:33
objA:28
objA:29
objA:30
objB:34
objA:31
objA:32
objB:35
objA:33
objA:34
objB:36
objA:35
objA:36
objB:37
objA:37
objA:38
objA:39
objA:40
objB:38
objB:39
objB:40
objA:41
objA:42
objA:43
objA:44
objA:45
objB:41
objA:46
objA:47
objA:48
objB:42
objA:49
objB:43
objA:50
objA:51
objA:52
objA:53
objA:54
objB:44
```

经检测，这已经达到我们所期望的结果。

