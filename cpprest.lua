project "cpprest"

dofile(_BUILD_DIR .. "/static_library.lua")

configuration { "*" }

uuid "55C73D78-9569-47CA-973D-9D26EC94D2BD"

defines {
  "_NO_PPLXIMP", -- prevent building a dynamic library
  "_NO_ASYNCRTIMP",
  "CPPREST_EXCLUDE_BROTLI",
  "CPPREST_EXCLUDE_WEBSOCKETS",
}

includedirs {
  "Release/src/pch",
  "Release/include",
  _3RDPARTY_DIR .. "/boost",
  _3RDPARTY_DIR .. "/zlib",
}

files {
  "Release/include/**.h",
  "Release/src/pplx/pplx.cpp",
  "Release/src/pplx/pplxtasks.cpp",
  "Release/src/uri/uri.cpp",
  "Release/src/uri/uri_builder.cpp",
  "Release/src/utilities/asyncrt_utils.cpp",
}

local t_httpfiles = {
  "Release/src/http/client/http_client.cpp",
  "Release/src/http/client/http_client_msg.cpp",
  "Release/src/http/common/http_compression.cpp",
  "Release/src/http/common/http_helpers.cpp",
  "Release/src/http/common/http_msg.cpp",
  "Release/src/http/oauth/oauth1.cpp",
  "Release/src/http/oauth/oauth2.cpp",
  "Release/src/json/json.cpp",
  "Release/src/json/json_parsing.cpp",
  "Release/src/json/json_serialization.cpp",
  "Release/src/utilities/base64.cpp",
  "Release/src/utilities/web_utilities.cpp",
}


if (_PLATFORM_ANDROID) then
  includedirs {
    _3RDPARTY_DIR .. "/openssl/include",
  }

  files {
    t_httpfiles,
    "Release/src/http/client/http_client_asio.cpp",
    "Release/src/http/client/x509_cert_utilities.cpp",
    "Release/src/pplx/pplxlinux.cpp",
    "Release/src/pplx/threadpool.cpp",
  }
end

if (_PLATFORM_COCOA) then
  files {
    "Release/src/pplx/pplxapple.cpp",
  }
end

if (_PLATFORM_IOS) then
  files {
    "Release/src/pplx/pplxapple.cpp",
  }
end

if (_PLATFORM_LINUX) then
  includedirs {
    _3RDPARTY_DIR .. "/openssl/include",
  }

  files {
    t_httpfiles,
    "Release/src/http/client/http_client_asio.cpp",
    "Release/src/http/client/x509_cert_utilities.cpp",
    "Release/src/pplx/pplxlinux.cpp",
    "Release/src/pplx/threadpool.cpp",
  }
end

if (_PLATFORM_MACOS) then
  files {
    "Release/src/pplx/pplxapple.cpp",
  }
end

if (_PLATFORM_WINDOWS) then
  defines {
    "CPPREST_FORCE_PPLX",
  }

  files {
    t_httpfiles,
    "Release/src/http/client/http_client_winhttp.cpp",
    "Release/src/pplx/pplxwin.cpp",
  }
end

if (_PLATFORM_WINUWP) then
  defines {
    "CPPREST_FORCE_PPLX",
  }

  files {
    t_httpfiles,
    "Release/src/http/client/http_client_winrt.cpp",
    "Release/src/pplx/pplxwin.cpp",
  }

  buildoptions {
    "/ZW",
    "/AI\"$(VCIDEInstallDir)vcpackages\"",
    "/AI\"$(WindowsSDK_UnionMetadataPath)\"",
  }
end
