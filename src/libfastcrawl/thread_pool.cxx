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

#include "thread_pool.hxx"
#include "utility.hxx"


namespace fastcrawl {

size_t thread_pool::size(size_t thread_cnt) {
    std::lock_guard<std::mutex> thread_list_lock(m_thread_list_mutex);

    // Start missing threads
    const size_t thread_list_size = m_thread_list.size();
    if (thread_list_size < thread_cnt)
        start_thread_impl(thread_cnt - thread_list_size);

    return m_thread_list.size();
}


bool thread_pool::run(thread_pool::job_t job) {
    size_t tbusy;

    {
        std::lock_guard<std::mutex> job_queue_lock(m_job_queue_mutex);

        if (m_shutdown) return false;  // no more jobs accepted

        // Push job to job queue
        m_job_queue.push(job);
        m_job_ready.notify_one();

        tbusy = m_tbusy;
    }

    // Check if another tread should be started
    std::lock_guard<std::mutex> thread_list_lock(m_thread_list_mutex);
    if (m_thread_list.size() == tbusy) start_thread_impl();

    return true;
}


void thread_pool::shutdown() {
    {
        std::lock_guard<std::mutex> job_queue_lock(m_job_queue_mutex);

        if (m_shutdown) return;  // already down

        // Signalise shutdown to threads
        m_shutdown = true;
        m_job_ready.notify_all();
    }

    // Join threads
    std::lock_guard<std::mutex> thread_list_lock(m_thread_list_mutex);

    for (auto & thread: m_thread_list)
        thread.join();
}


size_t thread_pool::start_thread_impl(size_t tcnt) {
    const size_t thread_list_size = m_thread_list.size();

    // Apply thread limit
    if (thread_list_size + tcnt > m_tmax)
        tcnt = m_tmax - thread_list_size;

    // Start threads
    for (size_t i = 0; i < tcnt; ++i)
        m_thread_list.emplace_back(
            std::bind(&thread_pool::routine, this));

    return tcnt;
}


void thread_pool::routine() {
    std::unique_lock<std::mutex> job_queue_lock(m_job_queue_mutex);

    while (!m_shutdown) {
        // Execute queued jobs
        while (m_job_queue.size()) {
            auto job = m_job_queue.front();
            m_job_queue.pop();

            ++m_tbusy;
            job_queue_lock.unlock();

            run_at_eos(([this, &job_queue_lock]() {
                job_queue_lock.lock();
                --m_tbusy;
            }));

            job();  // run job
        }

        if (m_shutdown) break;  // shutdown was signalised while executing jobs

        m_job_ready.wait(job_queue_lock);  // wait for job
    }
}

}  // end of namespace fastcrawl
