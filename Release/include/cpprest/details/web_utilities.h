/***
 * Copyright (C) Microsoft. All rights reserved.
 * Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
 *
 * =+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
 *
 * utility classes used by the different web:: clients
 *
 * =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 ****/
#pragma once

#include "cpprest/asyncrt_utils.h"
#include "cpprest/uri.h"

#include <windows.storage.streams.h>

namespace web
{
namespace details
{
class zero_memory_deleter
{
public:
    _ASYNCRTIMP void operator()(::utility::string_t* data) const;
};
typedef std::unique_ptr<::utility::string_t, zero_memory_deleter> plaintext_string;
} // namespace details

/// <summary>
/// web_proxy represents the concept of the web proxy, which can be auto-discovered,
/// disabled, or specified explicitly by the user.
/// </summary>
class web_proxy
{
    enum web_proxy_mode_internal
    {
        use_default_,
        use_auto_discovery_,
        disabled_,
        user_provided_
    };

public:
    enum web_proxy_mode
    {
        use_default = use_default_,
        use_auto_discovery = use_auto_discovery_,
        disabled = disabled_
    };

    /// <summary>
    /// Constructs a proxy with the default settings.
    /// </summary>
    web_proxy() : m_address(_XPLATSTR("")), m_mode(use_default_) {}

    /// <summary>
    /// Creates a proxy with specified mode.
    /// </summary>
    /// <param name="mode">Mode to use.</param>
    web_proxy(web_proxy_mode mode) : m_address(_XPLATSTR("")), m_mode(static_cast<web_proxy_mode_internal>(mode)) {}

    /// <summary>
    /// Creates a proxy explicitly with provided address.
    /// </summary>
    /// <param name="address">Proxy URI to use.</param>
    web_proxy(uri address) : m_address(address), m_mode(user_provided_) {}

    /// <summary>
    /// Gets this proxy's URI address. Returns an empty URI if not explicitly set by user.
    /// </summary>
    /// <returns>A reference to this proxy's URI.</returns>
    const uri& address() const { return m_address; }

    /// <summary>
    /// Checks if this proxy was constructed with default settings.
    /// </summary>
    /// <returns>True if default, false otherwise.</param>
    bool is_default() const { return m_mode == use_default_; }

    /// <summary>
    /// Checks if using a proxy is disabled.
    /// </summary>
    /// <returns>True if disabled, false otherwise.</returns>
    bool is_disabled() const { return m_mode == disabled_; }

    /// <summary>
    /// Checks if the auto discovery protocol, WPAD, is to be used.
    /// </summary>
    /// <returns>True if auto discovery enabled, false otherwise.</returns>
    bool is_auto_discovery() const { return m_mode == use_auto_discovery_; }

    /// <summary>
    /// Checks if a proxy address is explicitly specified by the user.
    /// </summary>
    /// <returns>True if a proxy address was explicitly specified, false otherwise.</returns>
    bool is_specified() const { return m_mode == user_provided_; }

private:
    web::uri m_address;
    web_proxy_mode_internal m_mode;
};

} // namespace web
