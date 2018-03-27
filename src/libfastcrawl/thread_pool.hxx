#ifndef fastcrawl__thread_pool_hxx
#define fastcrawl__thread_pool_hxx

#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <functional>
#include <list>
#include <cstdint>


namespace fastcrawl {

class thread_pool {
    public:

    using job_t = std::function<void ()>;

    private:

    using thread_list_t = std::list<std::thread>;
    using job_queue_t   = std::queue<job_t>;

    const size_t                    m_tmin;
    const size_t                    m_tmax;
    size_t                          m_tbusy;
    bool                            m_shutdown;
    thread_list_t                   m_thread_list;
    job_queue_t                     m_job_queue;
    mutable std::mutex              m_thread_list_mutex;
    mutable std::mutex              m_job_queue_mutex;
    mutable std::condition_variable m_job_ready;

    public:

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

    size_t size() const {
        std::lock_guard<std::mutex> thread_list_lock(m_thread_list_mutex);
        return m_thread_list.size();
    }

    size_t size(size_t);

    size_t busy() const {
        std::lock_guard<std::mutex> job_queue_lock(m_job_queue_mutex);
        return m_tbusy;
    }

    size_t start_thread(size_t tcnt = 1) {
        std::lock_guard<std::mutex> thread_list_lock(m_thread_list_mutex);
        return start_thread_impl(tcnt);
    }

    bool run(job_t job);

    void shutdown();

    ~thread_pool() { shutdown(); }

    private:

    size_t start_thread_impl(size_t tcnt = 1);

    void routine();

};  // end of class thread_pool

}  // end of namespace fastcrwal

#endif  // end of fastcrawl__thread_pool_hxx
