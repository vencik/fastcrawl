#ifndef fastcrawl__utility_hxx
#define fastcrawl__utility_hxx

/**
 *  \file
 *  \brief  Various utilities
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


#define CONCAT_IMPL(a1, a2) a1 ## a2
#define CONCAT(a1, a2) CONCAT_IMPL(a1, a2)


/**
 *  \brief  Run action at end-of-scope
 *
 *  Executes code (typically a short lambda) at end of scope.
 *  It's done by instantiating a \ref deferred_action instance.
 *
 *  \param  fn  Typically lambda or \c std::function or a functor/plain function
 */
#define run_at_eos(fn) \
    auto CONCAT(__fastcrawl__utility__deferred_, __LINE__) = \
        fastcrawl::deferred(fn)


namespace fastcrawl {

/**
 *  \brief  Deferred action
 *
 *  The object executes an action upon its destruction, i.e. at the end
 *  of its definition scope.
 *
 *  \tparam  Fn  Action type (lambda, std::function, functor, plain function)
 */
template <class Fn>
class deferred_action {
    private:

    Fn m_fn;  /**< Action */

    public:

    /** Constructor (taking the action to execute later) */
    deferred_action(Fn fn): m_fn(fn) {}

    /** Destructor (executes the action) */
    ~deferred_action() { m_fn(); }

};  // end of template class deferred_action


/**
 *  \brief  Convenience \ref deferred_action construction function
 *
 *  Constructs \ref deferred_action object.
 *  Allows for action type inferrence.
 *
 *  \param  fn  Action
 *
 *  \return Deferred action object
 */
template <class Fn>
inline deferred_action<Fn> deferred(Fn fn) {
    return deferred_action<Fn>(fn);
}

}  // end of namespace fastcrawl

#endif  // end of #ifndef fastcrawl__utility_hxx
