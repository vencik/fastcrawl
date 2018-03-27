#ifndef fastcrawl__online_data_processor_hxx
#define fastcrawl__online_data_processor_hxx

/**
 *  \file
 *  \brief  Online data processor interface & helpers
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

#include <utility>
#include <cstddef>


namespace fastcrawl {

/**
 *  \brief  Online data processor interface
 *
 *  Interface for injections that process content data incrementally
 *  during \ref download.
 */
class online_data_processor {
    public:

    /**
     *  \brief  Process data chunk
     *
     *  Provides implementation with another continuous data chunk.
     *
     *  \param  data  Data chunk
     *  \param  size  Data chunk size
     */
    virtual void operator () (unsigned char * data, size_t size) = 0;

    virtual ~online_data_processor() {}

};  // end of class online_data_processor


/**
 *  \brief  Compound online data processor
 *
 *  Constructs immutable sequence of online data processors.
 *  The result is an online data processor as well.
 *  The processors are provided data chunks one by one.
 *
 *  \tparam  Proc   1st online data processor type
 *  \tparam  Procs  Next online data processors types
 */
template <class Proc, class... Procs>
class compound_data_processor: public online_data_processor {
    private:

    /** Next processors compound type */
    using subproc_t = compound_data_processor<Procs...>;

    Proc      m_proc;   /**< 1st online data processor   */
    subproc_t m_procs;  /**< Next online data processors */

    public:

    /**
     *  \brief  Constructor
     *
     *  \param  proc   1st online data processor
     *  \param  procs  Next online data processors
     */
    compound_data_processor(Proc && proc, Procs &&... procs):
        m_proc(std::forward<Proc>(proc)),
        m_procs(std::forward<Procs>(procs)...)
    {}

    /** Implements \ref online_data_processor::operator() */
    void operator () (unsigned char * data, size_t size) {
        m_proc(data, size);
        m_procs(data, size);
    }

};  // end of template class compound_data_processor

/** Compound online data processor (template recursion fixed point) */
template <class Proc>
class compound_data_processor<Proc>: public online_data_processor {
    private:

    Proc m_proc;  /**< Last online data processor */

    public:

    /** Constructor */
    compound_data_processor(Proc && proc):
        m_proc(std::forward<Proc>(proc))
    {}

    /** Implements \ref online_data_processor::operator() */
    void operator () (unsigned char * data, size_t size) {
        m_proc(data, size);
    }

};  // end of template class compound_data_processor specialisation


/**
 *  \brief  Compound online data processor construction convenence function
 *
 *  Constructs compound online data processor.
 *  Allows for processors' types inference.
 *
 *  \param  proc   1st online data processor
 *  \param  procs  Next online data processors
 *
 *  \return Compound online data processor
 */
template <class Proc, class... Procs>
compound_data_processor<Proc, Procs...> data_processor(
    Proc  &&     proc,
    Procs &&... procs)
{
    return compound_data_processor<Proc, Procs...>(
        std::forward<Proc>(proc),
        std::forward<Procs>(procs)...);
}

}  // end of namespace fastcrawl

#endif  // end of #ifndef fastcrawl__online_data_processor_hxx
