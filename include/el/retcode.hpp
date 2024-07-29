/*
ELEKTRON © 2022
Written by melektron
www.elektron.work
27.12.22, 13:44
All rights reserved.

This source code is licensed under the Apache-2.0 license found in the
LICENSE file in the root directory of this source tree. 

Enumerations for function return values used to more clearly describe the outcome
of a function.
*/

#pragma once

#define EL_RETURN_IF_NOT_OK(expression) {el::retcode __retval; if ((__retval = (expression)) != el::retcode::ok) return __retval;}

namespace el
{
    enum class retcode
    {
        ok,
        err,
        timeout,
        tx_decline,
        invalid,    // general invalid state (like invalid data structure)
        inv_path,   // invalid file path
        inv_resp,   // invalid response
        inv_index,  // index doesn't exist
        tx_err,
        nak,        // negative acknowledgement
        exit_sig,   // exit signal (not an error signal)
        empty,
        gshutdown,  // graceful shutdown
        fshutdown,  // forceful shutdown
        noimpl,     // no implementation (virtual function should be overloaded but isn't)
        noperm,     // no permission
        nolock,
        notfound,   // resource not found (e.g. file not found)
        repaired,   // data structure or invalid state repaired
        busy,       // device busy
        e_size,     // size error (e.g. buffer too small)
    };

    inline const char *retcode_name(retcode _ret)
    {
        switch (_ret)
        {
        case retcode::ok: return "[ok]";
        case retcode::err: return "[err]";
        case retcode::inv_index: return "[inv_index]";
        case retcode::invalid: return "[invalid]";
        case retcode::timeout: return "[timeout]";
        case retcode::tx_decline: return "[tx_decline]";
        case retcode::inv_resp: return "[inv_resp]";
        case retcode::tx_err: return "[tx_err]";
        case retcode::nak: return "[nak]";
        case retcode::exit_sig: return "[exit_sig]";
        case retcode::empty: return "[empty]";
        case retcode::gshutdown: return "[gshutdown]";
        case retcode::fshutdown: return "[fshutdown]";
        case retcode::noimpl:  return "[noimpl]:";
        case retcode::noperm: return "[noperm]";
        case retcode::nolock: return "[nolock]";
        case retcode::notfound: return "[notfound]";
        case retcode::repaired: return "[repaired]";
        case retcode::busy: return "[busy]";
        default: return "[?]";
        }
    }
}
