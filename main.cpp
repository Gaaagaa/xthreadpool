/**
 * @file    main.cpp
 * <pre>
 * Copyright (c) 2018, Gaaagaa All rights reserved.
 * 
 * 文件名称：main.cpp
 * 创建日期：2018年12月13日
 * 文件标识：
 * 文件摘要：x_threadpool_t 线程池的测试程序。
 * 
 * 当前版本：1.0.0.0
 * 作    者：
 * 完成日期：2018年12月13日
 * 版本摘要：
 * 
 * 取代版本：
 * 原作者  ：
 * 完成日期：
 * 版本摘要：
 * </pre>
 */

#include "xthreadpool.h"
#include <iostream>
#include <string>
#include <chrono>
#include <stdio.h>

////////////////////////////////////////////////////////////////////////////////

/**********************************************************/
/**
 * @brief C 函数模式的任务接口。
 * 
 * @param [in ] task_id   : 任务标识 ID。
 * @param [in ] task_iter : 任务传递的测试值。
 * 
 */
void func_task(int task_id, int task_iter)
{
	int count = 1;
	do
	{
		printf("func_task[%d, %d] => count: %d\n", task_id, task_iter, count);
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	} while (count++ < 10);
}

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

////////////////////////////////////////////////////////////////////////////////

int main(int argc, char * argv[])
{
	int nstep = 0;

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
#if 0
	nstep += 0;
	for (int iter = nstep; iter < (nstep + 100); iter += 10)
		xht_pool.submit_task_ex(func_task, iter, iter * iter);
#endif
	//======================================
	// 提交任务对象 : lambda 表达式
#if 0
	nstep += 100;
	for (int iter = nstep; iter < (nstep + 100); iter += 10)
		xht_pool.submit_task_ex(
			[iter](x_running_checker_t * xchecker_ptr) -> void    // 带回调检测
			{
				int task_id = iter;
				for (int jter = 0; (jter < 10) && xchecker_ptr->is_enable_running(); ++jter)
				{
					printf("rchecker[%d] lambda A task id : %d -> %d\n", (int)xchecker_ptr->thread_index(), task_id, task_id + jter);
					std::this_thread::sleep_for(std::chrono::milliseconds(100));
				}
			},
			x_running_checker_t::xholder());
#endif
#if 0
	nstep += 100;
	for (int iter = nstep; iter < (nstep + 100); iter += 10)
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


	nstep += 100;
	for (int iter = nstep; iter < (nstep + 100); iter += 10)
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
#endif
	//======================================
	// 延时测试
#if 1
	nstep += 100;
	for (int iter = nstep; iter < (nstep + 100); iter += 10)
	{
		xht_pool.submit_task_ex(
			[iter](std::chrono::system_clock::time_point timestamp) -> void
			{
				std::chrono::microseconds dtime =
					std::chrono::duration_cast< std::chrono::microseconds >(std::chrono::system_clock::now() - timestamp);
				printf("delay time: %d, %ld us\n", iter, dtime.count());
			},
			std::chrono::system_clock::now());

		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
#endif
	//======================================
	// 提交任务对象 : 仿函数对象
#if 0
	nstep += 100;
	for (int iter = nstep; iter < (nstep + 100); iter += 10)
		xht_pool.submit_task_ex((functor_task_A(iter)));

	nstep += 100;
	for (int iter = nstep; iter < (nstep + 100); iter += 10)
		xht_pool.submit_task_ex((functor_task_B(iter)), iter * iter / 2);
#endif
	//======================================
	// 提交任务对象 : 类对象的成员函数调用
#if 0
	memfunc_task mftask(nstep);  // 注意，这个栈区对象的生命期需要在线程池关闭前存活
	nstep += 100;
	for (int iter = nstep; iter < (nstep + 100); iter += 10)
	{
		xht_pool.submit_task_ex(&memfunc_task::memfunc, &mftask, iter);
	}
#endif
	//======================================
	// 提交任务对象 : 重载的任务对象
#if 0
	nstep += 100;
	for (int iter = nstep; iter < (nstep + 100); iter += 10)
		xht_pool.submit_task((x_task_ptr_t)(new user_task(iter)));
#endif
	//======================================
	// 测试动态调整工作线程数量
#if 0
	while (xht_pool.task_count() > 50)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1));

		if ((xht_pool.task_count() < 100) && (xht_pool.size() > 4))
		{
			printf("======================================> before resize : %d, %d\n",
				(int)xht_pool.size(), (int)xht_pool.task_count());

			auto time1 = std::chrono::system_clock::now();
			xht_pool.resize(xht_pool.size() / 2);
			auto time2 = std::chrono::system_clock::now();

			std::chrono::microseconds dtime = std::chrono::duration_cast< std::chrono::microseconds >(time2 - time1);

			printf("======================================>[%ld] after  resize : %d, %d\n",
				dtime.count(), (int)xht_pool.size(), (int)xht_pool.task_count());
		}
	}

	if (xht_pool.task_count() > 20)
	{
			printf("======================================> before resize : %d, %d\n",
				(int)xht_pool.size(), (int)xht_pool.task_count());

			auto time1 = std::chrono::system_clock::now();
			xht_pool.shutdown();
			auto time2 = std::chrono::system_clock::now();

			std::chrono::microseconds dtime = std::chrono::duration_cast< std::chrono::microseconds >(time2 - time1);

			printf("======================================>[%ld] after  resize : %d, %d\n",
				dtime.count(), (int)xht_pool.size(), (int)xht_pool.task_count());

			xht_pool.cleanup_task();
			printf("======================================> after cleanup_task : %d, %d\n",
				(int)xht_pool.size(), (int)xht_pool.task_count());
	}
#endif
	//======================================

	// 等待所有任务执行完成
	while (xht_pool.task_count() > 0)
		std::this_thread::sleep_for(std::chrono::milliseconds(1));

	// 关闭线程池
	xht_pool.shutdown();

    return 0;
}
