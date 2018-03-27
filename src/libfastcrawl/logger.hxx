#ifndef fastcrawl__logger_hxx
#define fastcrawl__logger_hxx

/**
 *  \file
 *  \brief  Logging
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

#include <iostream>


/** Log stream for \ref logger instances (log always level) */
#define LOG std::cerr


/**
 *  \brief  Log stream for \ref logger instances (verbose level)
 *
 *  Using this macro, logging will only be done if verbose level is set.
 */
#define VLOG if (!m_vlog); else std::cerr


namespace fastcrawl {

/**
 *  \brief  Simple logger
 *
 *  Descendants of the class are capable of using the logging macros
 *  \ref LOG and \ref VLOG.
 */
class logger {
    protected:

    bool m_vlog;  /**< Verbose logging flag */

    public:

    logger(): m_vlog(false) {}

    /** Verbose logging flag getter */
    bool verbose_log() const { return m_vlog; }

    /** Verbose logging flag setter */
    void verbose_log(bool verbose) { m_vlog = verbose; }

};  // end of class logger

}  // end of namespace fastcrawl

#endif  // end of #ifndef fastcrawl__logger_hxx
