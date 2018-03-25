#ifndef fastcrawl__utility_hxx
#define fastcrawl__utility_hxx


#define CONCAT_IMPL(a1, a2) a1 ## a2
#define CONCAT(a1, a2) CONCAT_IMPL(a1, a2)


#define run_at_eos(fn) \
    auto CONCAT(__fastcrawl__utility__deferred_, __LINE__) = \
        fastcrawl::deferred(fn)


namespace fastcrawl {

template <class Fn>
class deferred_action {
    private:

    Fn m_fn;

    public:

    deferred_action(Fn fn): m_fn(fn) {}

    ~deferred_action() { m_fn(); }

};  // end of template class deferred_action


template <class Fn>
inline deferred_action<Fn> deferred(Fn fn) {
    return deferred_action<Fn>(fn);
}

}  // end of namespace fastcrawl

#endif  // end of #ifndef fastcrawl__utility_hxx
