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
    std::lock_guard<std::mutex> job_queue_lock(m_job_queue_mutex);

    if (m_shutdown) return false;  // no more jobs accepted

    // Push job to job queue
    m_job_queue.push(job);
    m_job_ready.notify_one();
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

            job_queue_lock.unlock();
            run_at_eos([&job_queue_lock]() { job_queue_lock.lock(); });

            job();
        }

        if (m_shutdown) break;  // shutdown was signalised while executing jobs

        m_job_ready.wait(job_queue_lock);  // wait for job
    }
}

}  // end of namespace fastcrawl
