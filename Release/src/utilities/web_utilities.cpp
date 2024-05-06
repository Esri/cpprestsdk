/***
 * Copyright (C) Microsoft. All rights reserved.
 * Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
 *
 * =+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
 *
 * Credential and proxy utilities.
 *
 * For the latest on this and related APIs, please see: https://github.com/Microsoft/cpprestsdk
 *
 * =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 ****/

#include "stdafx.h"

#include <assert.h>

#if defined(_WIN32)
#include <robuffer.h>
#include <windows.security.cryptography.dataprotection.h>
#endif

namespace web
{
namespace details
{
void zero_memory_deleter::operator()(::utility::string_t* data) const
{
    CASABLANCA_UNREFERENCED_PARAMETER(data);
#if defined(_WIN32)
    SecureZeroMemory(&(*data)[0], data->size() * sizeof(::utility::string_t::value_type));
    delete data;
#endif
}
} // namespace details

} // namespace web
