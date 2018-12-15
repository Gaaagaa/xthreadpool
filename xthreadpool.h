/**
 * @file    xthreadpool.h
 * <pre>
 * Copyright (c) 2018, Gaaagaa All rights reserved.
 * 
 * 文件名称：xthreadpool.h
 * 创建日期：2018年12月12日
 * 文件标识：
 * 文件摘要：使用 C++11 新标准 thread 线程对象实现的线程池类。
 * 
 * 当前版本：1.0.0.0
 * 作    者：
 * 完成日期：2018年12月12日
 * 版本摘要：
 * 
 * 取代版本：
 * 原作者  ：
 * 完成日期：
 * 版本摘要：
 * </pre>
 */

#ifndef __XTHREADPOOL_H__
#define __XTHREADPOOL_H__

#include <list>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <functional>
#include <thread>
#include <atomic>
#include <utility>
#include <type_traits>

////////////////////////////////////////////////////////////////////////////////

/**
 * @brief 命名空间内定义一些 std::tuple 相关操作的辅助接口。
 */
namespace nstuple
{

////////////////////////////////////////////////////////////////////////////////

/**
 * Stores a tuple of indices. Used by bind() to extract the elements in a tuple. 
 */
template< size_t... _Indexes >
struct X_Index_tuple
{

};

/// Builds an X_Index_tuple< 0, 1, 2, ..., _Num - 1 >.
template< size_t _Num, typename _Tuple = X_Index_tuple<> >
struct X_Build_index_tuple;

template< size_t _Num, size_t... _Indexes >
struct X_Build_index_tuple< _Num, X_Index_tuple< _Indexes... > >
    : X_Build_index_tuple< _Num - 1, X_Index_tuple< _Indexes..., sizeof...(_Indexes) > >
{

};

template< size_t... _Indexes >
struct X_Build_index_tuple< 0, X_Index_tuple< _Indexes... > >
{
    typedef X_Index_tuple< _Indexes... > __type;
};

/**
 * @brief 
 * <pre>
 *   本区段的代码改写自 gcc 8.2.0 版内的 STL 源码（functional 文件）。
 *   主要用于 std::tuple 对象的参数解包操作，可参看如下示例代码。
 * </pre>
 * 
 * @code
 * <pre>
 * #include <tuple>
 * #include <iostream>
 * 
 * void call_test(int v1, float v2, double v3)
 * {
 *     std::cout << "(v1, v2, v3) : "
 *               << "("  << v1
 *               << ", " << v2
 *               << ", " << v3
 *               << ")"  << std::endl;
 * }
 * 
 * using X_Tuple   = std::tuple< int, float, double >;
 * using X_Indices = nstuple::X_Build_index_tuple< std::tuple_size< X_Tuple >::value >::__type;
 * 
 * template< size_t... _Ind >
 * void _S_Invoke(X_Tuple && xtuple, nstuple::X_Index_tuple< _Ind... > x_unused_object)
 * {
 *     call_test(std::get< _Ind >(std::move(xtuple))...);
 * }
 * 
 * int main(int argc, char * argv[])
 * {
 *     X_Tuple xtuple{ 1234, 1.234F, 1.234 };
 *     _S_Invoke(std::forward< X_Tuple >(xtuple), X_Indices());
 * 
 *     return 0;
 * }
 * </pre>
 */

////////////////////////////////////////////////////////////////////////////////

/**
 * @struct X_check
 * @brief  协助 X_has_type() 进行类型检查。
 */
template< bool... >
struct X_type_check
{

};

/**
 * @brief 判断 tuple 对象内是否包含某个数据类型。
 */
template< typename _Fy, typename... _Ty >
constexpr bool X_has_type(const std::tuple< _Ty... > & x_unused_object)
{
    return !std::is_same< X_type_check< false, std::is_same< _Fy, _Ty >::value... >,
                          X_type_check< std::is_same< _Fy, _Ty >::value..., false > >::value;
}

/**
 * @brief X_has_type() 接口的测试代码如下所示。
 * @code
 * <pre>
 * #include <type_traits>
 * #include <tuple>
 * #include <iostream>
 * 
 * int main(int argc, char * argv[])
 * {
 *     std::cout << std::boolalpha;
 *     std::cout << nstuple::X_has_type< int    >(std::tuple< int, char, double >{ 100, 'a', 3.1415926 }) << std::endl;
 *     std::cout << nstuple::X_has_type< char   >(std::tuple< int, char, double >{ 100, 'a', 3.1415926 }) << std::endl;
 *     std::cout << nstuple::X_has_type< double >(std::tuple< int, char, double >{ 100, 'a', 3.1415926 }) << std::endl;
 *     std::cout << nstuple::X_has_type< float  >(std::tuple< int, char, double >{ 100, 'a', 3.1415926 }) << std::endl;
 *     std::cout << nstuple::X_has_type< void   >(std::tuple< int, char, double >{ 100, 'a', 3.1415926 }) << std::endl;
 * 
 *     return 0;
 * }
 * </pre>
 */

////////////////////////////////////////////////////////////////////////////////

/** X_type_count 的前置声明 */
template< typename _Fy, typename... _Ty >
struct X_type_count;

/**
 * @struct X_type_count< _Fy, _Hy, _Ty... >
 * @brief  递归的数据类型统计类，协助 X_type_nums() 接口类型数量计数。
 */
template< typename _Fy, typename _Hy, typename... _Ty >
struct X_type_count< _Fy, _Hy, _Ty... >
{
    enum { value = (int)(std::is_same< _Fy, _Hy >::value) + X_type_count< _Fy, _Ty... >::value };
};

/**
 * @struct X_type_count< _Fy, _Hy >
 * @brief  终止递归的数据类型统计类，协助 X_type_nums() 接口类型数量计数。
 */
template< typename _Fy >
struct X_type_count< _Fy >
{
    enum { value = 0 };
};

/**
 * @brief 统计 tuple 对象内某个数据类型的数量。
 */
template< typename _Fy, typename... _Ty >
constexpr size_t X_type_nums(const std::tuple< _Ty... > & x_unused_object)
{
    return X_type_count< _Fy, _Ty... >::value;
}

/**
 * @brief X_type_nums() 接口的测试代码如下所示。
 * @code
 * <pre>
 * #include <type_traits>
 * #include <tuple>
 * #include <iostream>
 * 
 * int main(int argc, char * argv[])
 * {
 *     std::cout << nstuple::X_type_count< int, int, int, char, char, double >::value << std::endl;
 * 
 *     std::cout << nstuple::X_type_nums< int    >(std::tuple< int, int, int, char, char, double >{}) << std::endl;
 *     std::cout << nstuple::X_type_nums< char   >(std::tuple< int, int, int, char, char, double >{}) << std::endl;
 *     std::cout << nstuple::X_type_nums< double >(std::tuple< int, int, int, char, char, double >{}) << std::endl;
 *     std::cout << nstuple::X_type_nums< float  >(std::tuple< int, int, int, char, char, double >{}) << std::endl;
 *     std::cout << nstuple::X_type_nums< void * >(std::tuple< int, int, int, char, char, double >{}) << std::endl;
 * 
 *     return 0;
 * }
 * </pre>
 */

////////////////////////////////////////////////////////////////////////////////

}; // namaspace nstuple

////////////////////////////////////////////////////////////////////////////////
// x_threadpool_t

/**
 * @class x_threadpool_t
 * @brief 线程池类。
 */
class x_threadpool_t
{
    // constructor/destructor
public:
    explicit x_threadpool_t(void) noexcept
        : m_enable_running(false)
        , m_xthds_capacity(0)
        , m_task_count(0)
    {

    }

    virtual ~x_threadpool_t(void)
    {
        if (is_startup())
            shutdown();
        cleanup_task();
    }

    x_threadpool_t(const x_threadpool_t & xobject) = delete;
    x_threadpool_t(x_threadpool_t & xobject) = delete;
    x_threadpool_t(const x_threadpool_t && xobject) = delete;
    x_threadpool_t(x_threadpool_t && xobject) = delete;
    x_threadpool_t & operator=(const x_threadpool_t & xobject) = delete;
    x_threadpool_t & operator=(x_threadpool_t && xobject) = delete;

    // common data types
public:
    /** 前置声明 */
    struct x_running_checker_t;
    struct x_task_deleter_t;

    /**
     * @struct x_task_t
     * @brief  任务对象的抽象基类。
     * @note   参看 @see get_deleter() 接口的说明。
     */
    struct x_task_t
    {
        // constructor/destructor
    public:
        virtual ~x_task_t(void) { }

        // extensible interfaces
    public:
        /**********************************************************/
        /**
         * @brief 任务对象执行流程的抽象接口。
         */
        virtual void run(x_running_checker_t * xchecker_ptr) = 0;

        /**********************************************************/
        /**
         * @brief 获取任务对象的删除器，重载该接口，可实现自定义的任务对象回收方式。
         * 
         * @note
         * <pre>
         *   任务对象提交至线程池后，工作线程会调用任务对象的 run() 
         *   接口执行具体的任务操作流程，run() 接口返回后，
         *   线程池的工作线程会调用该接口，获取任务对象的删除器，
         *   然后使用 删除器 对 任务对象 进行资源回收操作。
         *   重载该接口，获取自定义的删除器对象，则可实现自定义的资源回收方式。
         * </pre>
         */
        virtual const x_task_deleter_t * get_deleter(void) const
        {
            return &x_threadpool_t::_S_task_common_deleter;
        }
    };

    /** 任务对象指针类型 */
    typedef x_task_t * x_task_ptr;

    /**
     * @struct x_running_checker_t
     * @brief  辅助 x_task_t 对象进行回调判断线程池是否可继续运行，以便于对任务执行流程进行终止操作。
     */
    struct x_running_checker_t final
    {
        friend class x_threadpool_t;

        // constructor/destructor
    private:
        x_running_checker_t(const x_threadpool_t * xthis_pool_ptr, size_t xthread_index)
            : m_this_pool_ptr(xthis_pool_ptr)
            , m_xthread_index(xthread_index)
        {
        }

        ~x_running_checker_t(void)
        {
        }

        x_running_checker_t(const x_running_checker_t & xobject) = delete;
        x_running_checker_t(x_running_checker_t & xobject) = delete;
        x_running_checker_t(const x_running_checker_t && xobject) = delete;
        x_running_checker_t(x_running_checker_t && xobject) = delete;
        x_running_checker_t & operator=(const x_running_checker_t & xobject) = delete;
        x_running_checker_t & operator=(x_running_checker_t && xobject) = delete;

        // common data types
    public:
        using x_holder_t = x_running_checker_t *;

        /**********************************************************/
        /**
         * @brief 返回 x_running_checker_t 的占位对象。
         * @note  参看 x_threadpool_t::submit_task_ex() 接口的使用说明。
         */
        static inline const x_holder_t xholder(void) { return nullptr; };

        // public interfaces
    public:
        /**********************************************************/
        /**
         * @brief 回调判断当前工作线程是否可继续运行。
         */
        inline bool is_enable_running(void) const
        {
            return (m_this_pool_ptr->is_enable_running() &&
                    m_this_pool_ptr->is_within_capacity(m_xthread_index));
        }

        /**********************************************************/
        /**
         * @brief 返回所属的线程索引号。
         */
        inline size_t thread_index(void) const
        {
            return m_xthread_index;
        }

        // data members
    private:
        const x_threadpool_t * m_this_pool_ptr;  ///< 所属的线程池对象
        const size_t           m_xthread_index;  ///< 所属的线程索引号
    };

    /**
     * @struct x_task_deleter_t
     * @brief  任务对象的删除器接口类。
     */
    struct x_task_deleter_t
    {
        // extensible interfaces
    public:
        /**********************************************************/
        /**
         * @brief 对 任务对象 进行资源回收操作（删除操作）。
         */
        virtual void delete_task(x_task_ptr xtask_ptr)
        {
            if (nullptr != xtask_ptr)
                delete xtask_ptr;
        }
    };

private:
    /** 任务对象的通用删除器 */
    static x_task_deleter_t _S_task_common_deleter;

private:
    /**
     * @struct x_task_bind_t
     * @brief  内部的任务对象实现类。
     */
    template< typename _Func >
    struct x_task_bind_t : public x_task_t
    {
        // constructor/destructor
    public:
        x_task_bind_t(_Func && xfunc) : _M_func(std::forward< _Func >(xfunc))
        {

        }

        // overrides
    public:
        /**********************************************************/
        /**
         * @brief 任务对象执行流程。
         */
        virtual void run(x_running_checker_t * xchecker_ptr) override
        {
            _M_func();
        }

        // data members
    protected:
        _Func _M_func;   ///< 任务对象执行流程的工作接口（函数对象）
    };

    /**
     * @struct x_task_tuple_t
     * @brief  内部的任务对象实现类（带 x_running_checker_t 回调检测对象）。
     */
    template< typename _Func, typename _Tuple >
    struct x_task_tuple_t : public x_task_t
    {
        using _Indices = typename nstuple::X_Build_index_tuple< std::tuple_size< _Tuple >::value >::__type;

        // constructor/destructor
    public:
        x_task_tuple_t(_Func && xfunc, _Tuple && xargs)
            : _M_func(std::forward< _Func >(xfunc))
            , _M_args(std::forward< _Tuple >(xargs))
        {

        }

        // overrides
    public:
        /**********************************************************/
        /**
         * @brief 任务对象执行流程。
         */
        virtual void run(x_running_checker_t * xchecker_ptr) override
        {
            try { invoke(xchecker_ptr, _Indices()); } catch (...) { }
        }

        // internal invoking
    private:
        /**********************************************************/
        /**
         * @brief 执行流程。
         */
        template< size_t... _Ind >
        void invoke(x_running_checker_t * xchecker_ptr, nstuple::X_Index_tuple< _Ind... >)
        {
#if __cplusplus > 201103L
            std::get< x_threadpool_t::x_running_checker_t::x_holder_t >(_M_args) = xchecker_ptr;
#else // __cplusplus <= 201103L
            std::get< 0 >(_M_args) = xchecker_ptr;
#endif // __cplusplus
            auto xinvoker = std::bind(std::forward< _Func >(_M_func), std::get< _Ind >(std::move(_M_args))...);
            xinvoker();
        }

        // data members
    private:
        _Func  _M_func;   ///< 任务对象执行流程的工作接口
        _Tuple _M_args;   ///< 回调的工作参数
    };

    // common invoking
private:
    /** 特化 x_task_maker_t<> 对象后可进行 make_task() 接口的选择。 */
    template< size_t > struct x_task_maker_t
    {

    };

    /**********************************************************/
    /**
     * @brief 以 bind 参数方式创建任务对象。
     */
    template< typename _Func, typename... _Args >
    static x_task_ptr make_task(const x_task_maker_t< 0 > & xmaker, _Func && xfunc, _Args && ... xargs)
    {
        auto xbinder = std::bind(std::forward< _Func >(xfunc), std::forward< _Args >(xargs)...);
        return (new x_task_bind_t< decltype(xbinder) >(std::forward< decltype(xbinder) >(xbinder)));
    }

    /**********************************************************/
    /**
     * @brief 以 tuple 参数方式创建任务对象。
     */
    template< typename _Func, typename... _Args >
    static x_task_ptr make_task(const x_task_maker_t< 1 > & xmaker, _Func && xfunc, _Args && ... xargs)
    {
        using _Tuple = typename std::tuple< typename std::decay< _Args >::type... >;
        _Tuple xtuple{ std::forward< _Args >(xargs)... };

        return (new x_task_tuple_t< _Func, _Tuple >(std::forward< _Func >(xfunc),
                                                    std::forward< _Tuple >(xtuple)));
    }

public:
    /**********************************************************/
    /**
     * @brief 返回硬件线程上下文的数量。
     */
    static size_t hardware_concurrency(void) noexcept
    {
        return std::thread::hardware_concurrency();
    }

    // public interfaces
public:
    /**********************************************************/
    /**
     * @brief 启动线程池。
     * 
     * @param [in ] xthds : 工作线程的数量（若为 0，将取 hardware_concurrency() 返回值的 2倍 + 1）。
     * 
     * @return bool
     *         - 成功，返回 true；
     *         - 失败，返回 false。
     */
    bool startup(size_t xthds = 0)
    {
        // 检测当前是否已经启动
        if (is_startup())
            return false;

        // 启动各个工作线程
        try
        {
            resize((0 != xthds) ? xthds : (2 * hardware_concurrency() + 1));
        }
        catch(...)
        {
            shutdown();
            return false;
        }

        return true;
    }

    /**********************************************************/
    /**
     * @brief 关闭线程池。
     */
    void shutdown(void)
    {
        resize(0);
    }

    /**********************************************************/
    /**
     * @brief 判断是否已经启动。
     */
    inline bool is_startup(void) const { return !m_lst_threads.empty(); }

    /**********************************************************/
    /**
     * @brief 返回 线程池对象是否可继续运行 的标识。
     */
    inline bool is_enable_running(void) const { return m_enable_running; }

    /**********************************************************/
    /**
     * @brief 返回工作线程数量。
     */
    inline size_t size(void) const { return m_lst_threads.size(); }

    /**********************************************************/
    /**
     * @brief 调整工作线程数量。
     */
    void resize(size_t xthds)
    {
        std::lock_guard< std::mutex > xautolock_thds(m_lock_thread);

        m_enable_running = (0 != xthds);
        m_xthds_capacity = xthds;

        if (xthds > m_lst_threads.size())
        {
            // 增加工作线程数量
            for (size_t xiter_index = m_lst_threads.size();  xiter_index < xthds; ++xiter_index)
            {
                m_lst_threads.push_back(std::thread([this](size_t xiter_index) -> void { thread_run(xiter_index); }, xiter_index));
            }
        }
        else if (m_lst_threads.size() > 0)
        {
            // 通知所有工作线程对象，检测退出事件
            {
                std::lock_guard< std::mutex > xautolock_task(m_lock_task);
                m_thds_notifier.notify_all();
            }

            // 递减工作线程数量
            while (m_lst_threads.size() > xthds)
            {
                std::thread & t = m_lst_threads.back();
                if (t.joinable())
                    t.join();
                m_lst_threads.pop_back();
            }
        }
    }

    /**********************************************************/
    /**
     * @brief 提交任务对象。
     */
    void submit_task(x_task_ptr xtask_ptr)
    {
        if (nullptr != xtask_ptr)
        {
            std::lock_guard< std::mutex > xautolock(m_lock_task);
            m_lst_tasks.push_back(xtask_ptr);
            m_task_count.fetch_add(1);

            m_thds_notifier.notify_one();
        }
    }

    /**********************************************************/
    /**
     * @brief 提交任务对象（支持 仿函数对象 与 lambda 表达式 等类函数的泛型接口）。
     * 
     * @note xfunc 的回调操作过程中，若需要进行运行时检测（判断工作线程是否可继续运行），
     * 可在 xargs 参数列表中添加一个（且最多一个） x_running_checker_t::x_holder_t 
     * 占位对象；当前代码版本，对于 C++11 标准，必须放在 xargs 参数列表的首个参数，而更
     * 高版本的 C++ 标准（C++14, C++17 or later）就不做这个限制，可在参数列表的任意位置。
     */
    template< typename _Func, typename... _Args >
    void submit_task_ex(_Func && xfunc, _Args && ... xargs)
    {
        constexpr size_t const xchecker_count =
                nstuple::X_type_count<
                    x_running_checker_t::x_holder_t,
                    typename std::decay< _Args >::type... >::value;

        static_assert(xchecker_count < 2, "Too many arguments [x_running_checker_t::xholder()]");

        submit_task(make_task(
                        x_task_maker_t< xchecker_count >(),
                        std::forward< _Func >(xfunc),
                        std::forward< _Args >(xargs)...));
    }

    /**********************************************************/
    /**
     * @brief 返回任务对象数量。
     */
    inline size_t task_count(void) const { return m_task_count; }

    /**********************************************************/
    /**
     * @brief 清除任务队列中所有的任务对象。
     */
    void cleanup_task(void)
    {
        x_task_ptr xtask_ptr = nullptr;

        std::lock_guard< std::mutex > xautolock(m_lock_task);

        while (m_lst_tasks.size() > 0)
        {
            xtask_ptr = m_lst_tasks.front();
            m_lst_tasks.pop_front();

            if (nullptr != xtask_ptr)
            {
                delete xtask_ptr;
                xtask_ptr = nullptr;
            }

            m_task_count.fetch_sub(1);
        }
    }

    // internal invoking
private:
    /**********************************************************/
    /**
     * @brief 从任务队列中获取任务对象。
     */
    x_task_ptr get_task(void)
    {
        x_task_ptr xtask_ptr = nullptr;

        {
            std::lock_guard< std::mutex > xautolock(m_lock_task);
            if (!m_lst_tasks.empty())
            {
                xtask_ptr = m_lst_tasks.front();
                m_lst_tasks.pop_front();
            }
        }

        return xtask_ptr;
    }

    /**********************************************************/
    /**
     * @brief 判断线程索引号是否超过 工作线程对象的上限数量。
     */
    inline bool is_overtop_capacity(size_t xthread_index) const
    {
        return (xthread_index >= m_xthds_capacity);
    }

    /**********************************************************/
    /**
     * @brief 判断线程索引号是否在工作线程对象的上限数量 之内。
     */
    inline bool is_within_capacity(size_t xthread_index) const
    {
        return (xthread_index < m_xthds_capacity);
    }

    /**********************************************************/
    /**
     * @brief 工作线程的执行流程。
     */
    void thread_run(size_t xthread_index)
    {
        x_running_checker_t xht_checker(this, xthread_index);

        x_task_ptr         xtask_ptr    = nullptr;
        x_task_deleter_t * xdeleter_ptr = nullptr;

        while (xht_checker.is_enable_running())
        {
            if (m_lst_tasks.size() <= 0)
            {
#if 0
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                continue;
#else
                std::unique_lock< std::mutex > xunique_locker(m_lock_task);
                m_thds_notifier.wait(xunique_locker,
                                     [this, &xht_checker]() -> bool
                                     {
                                         return ((m_lst_tasks.size() > 0) || (!xht_checker.is_enable_running()));
                                     });
#endif
            }

            if (!xht_checker.is_enable_running())
            {
                break;
            }

            xtask_ptr = get_task();
            if (nullptr == xtask_ptr)
            {
                continue;
            }

            if (xht_checker.is_enable_running())
            {
                xtask_ptr->run(&xht_checker);
            }

            xdeleter_ptr = const_cast< x_task_deleter_t * >(xtask_ptr->get_deleter());
            if (nullptr != xdeleter_ptr)
            {
                xdeleter_ptr->delete_task(xtask_ptr);
            }

            m_task_count.fetch_sub(1);
        }
    }

    // data members
private:
    volatile bool            m_enable_running;  ///< 工作线程继续运行的标识值
    std::mutex               m_lock_thread;     ///< 工作线程对象的队列的同步操作锁
    volatile size_t          m_xthds_capacity;  ///< 工作线程对象的上限数量
    std::list< std::thread > m_lst_threads;     ///< 工作线程对象的队列（列表）
    std::condition_variable  m_thds_notifier;   ///< 工作线程对象的通知器（条件变量）

    std::mutex               m_lock_task;       ///< 任务队列的同步操作锁
    std::list< x_task_ptr >  m_lst_tasks;       ///< 任务对象队列（列表）
    std::atomic< size_t >    m_task_count;      ///< 任务对象计数器
};

//====================================================================

#ifndef SELECTANY

#if (defined(_WIN32) || defined(_WIN64))
#define SELECTANY   __declspec(selectany)
#else // _WIN32 || _WIN64
#define SELECTANY   __attribute__((weak))
#endif // _WIN32 || _WIN64

#endif // SELECTANY

/* 任务对象的通用删除器 */
SELECTANY x_threadpool_t::x_task_deleter_t x_threadpool_t::_S_task_common_deleter;

//====================================================================

typedef x_threadpool_t::x_running_checker_t x_running_checker_t;
typedef x_threadpool_t::x_task_t            x_task_t;
typedef x_threadpool_t::x_task_deleter_t    x_task_deleter_t;
typedef x_threadpool_t::x_task_ptr          x_task_ptr;

////////////////////////////////////////////////////////////////////////////////

#endif // __XTHREADPOOL_H__
