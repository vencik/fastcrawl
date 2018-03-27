#ifndef fastcrawl__thread_pool_hxx
#define fastcrawl__thread_pool_hxx

/**
 *  \file
 *  \brief  Thread pool
 *
 *  \date   2018/03/27
 *  \author Vaclav Krpec  <vencik@razdva.cz>
 *
 *
 *  LEGAL NOTICE
 *
 *  Copyright (c) 2018, Vaclav Krpec
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *
 *  1. Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the distribution.
 *
 *  3. Neither the name of the copyright holder nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 *  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 *  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER
 *  OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 *  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 *  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 *  OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 *  WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 *  OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 *  EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <functional>
#include <list>
#include <cstdint>


namespace fastcrawl {

/**
 *  \brief  Simple thread pool with automatic thread addition
 *
 *  The pool keeps ready threads that execute jobs from a queue.
 *  The jobs are \c std::function objects without arguments and return value.
 *  The pool size (i.e. number of pooled threads) may be limited to avoid
 *  excessive thread creation.
 *
 *  When a new job is being pushed to the job queue, thread availability
 *  is checked.
 *  If all threads are currently busy, another thread is started pro-actively
 *  unless thread limit is reached.
 */
class thread_pool {
    public:

    using job_t = std::function<void ()>;  /**< Thread job type */

    private:

    using thread_list_t = std::list<std::thread>;   /**< Thread list type */
    using job_queue_t   = std::queue<job_t>;        /**< Job queue type   */

    const size_t                    m_tmin;         /**< Pre-started threads */
    const size_t                    m_tmax;         /**< Thread limit        */
    size_t                          m_tbusy;        /**< Busy threads count  */
    bool                            m_shutdown;     /**< Pool shutdown flag  */
    thread_list_t                   m_thread_list;  /**< Pooled threads list */
    job_queue_t                     m_job_queue;    /**< Job queue           */

    // MT sync
    mutable std::mutex              m_thread_list_mutex;
    mutable std::mutex              m_job_queue_mutex;
    mutable std::condition_variable m_job_ready;

    public:

    /**
     *  \brief  Constructor
     *
     *  \param  tmin  Number of threads available in the pool from the start
     *  \param  tmax  Max. amount of threads in the pool
     */
    thread_pool(
        size_t tmin,
        size_t tmax = SIZE_MAX)
    :
        m_tmin(tmin),
        m_tmax(tmax),
        m_tbusy(0),
        m_shutdown(false)
    {
        start_thread(m_tmin);
    }

    /** Thread pool size (amount of threads) getter */
    size_t size() const {
        std::lock_guard<std::mutex> thread_list_lock(m_thread_list_mutex);
        return m_thread_list.size();
    }

    /** Thread pool size (amount of threads) setter */
    size_t size(size_t);

    /** Number of currently busy treads */
    size_t busy() const {
        std::lock_guard<std::mutex> job_queue_lock(m_job_queue_mutex);
        return m_tbusy;
    }

    /**
     *  \brief  Start another \c tcnt threads
     *
     *  Attempts to start \c tcnt new threads.
     *  Note that the amount of threads actually started may be lower,
     *  based on system resources and/or thread limit.
     *
     *  \param  tcnt  Number of threads to start
     *
     *  \return Number of started threads
     */
    size_t start_thread(size_t tcnt = 1) {
        std::lock_guard<std::mutex> thread_list_lock(m_thread_list_mutex);
        return start_thread_impl(tcnt);
    }

    /**
     *  \brief  Run \c job
     *
     *  \return \c true iff the \c job was queued
     */
    bool run(job_t job);

    /**
     *  \brief  Thread pool shudown
     *
     *  The job queue is closed; it won't accept any new jobs (see \ref run).
     *  The function will block until all jobs that were already queued before
     *  the call are executed.
     */
    void shutdown();

    /** Destructor (shuts the pool down, see \ref shutdown). */
    ~thread_pool() { shutdown(); }

    private:

    /** Implements \ref start_thread (no locking) */
    size_t start_thread_impl(size_t tcnt = 1);

    /** Pooled thread routine */
    void routine();

};  // end of class thread_pool

}  // end of namespace fastcrwal

#endif  // end of fastcrawl__thread_pool_hxx
