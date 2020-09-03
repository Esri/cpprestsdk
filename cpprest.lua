project "cpprest"

  local prj = project()
  local prjDir = prj.basedir

  -- -------------------------------------------------------------
  -- project
  -- -------------------------------------------------------------

  -- common project settings

  dofile (_BUILD_DIR .. "/3rdparty_static_project.lua")

  -- project specific settings

  uuid "55C73D78-9569-47CA-973D-9D26EC94D2BD"

  flags {
    "NoPCH",
  }

  defines {
    "_NO_PPLXIMP", -- prevent building a dynamic library
    "_NO_ASYNCRTIMP",
    "CPPREST_EXCLUDE_BROTLI",
    "CPPREST_EXCLUDE_WEBSOCKETS",
  }

  files {
    "Release/include/**.h",
    "Release/src/pplx/pplx.cpp",
    "Release/src/pplx/pplxtasks.cpp",
    "Release/src/uri/uri.cpp",
    "Release/src/uri/uri_builder.cpp",
    "Release/src/utilities/asyncrt_utils.cpp",
  }

  includedirs {
    "Release/src/pch",
    "Release/include",
    "../boost",
    "../zlib",
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

  -- -------------------------------------------------------------
  -- configurations
  -- -------------------------------------------------------------

  if (_PLATFORM_WINDOWS) then
    -- -------------------------------------------------------------
    -- configuration { "windows" }
    -- -------------------------------------------------------------

    -- common configuration settings

    dofile (_BUILD_DIR .. "/3rdparty_static_win.lua")

    -- project specific configuration settings

    configuration { "windows" }

      defines {
        "CPPREST_FORCE_PPLX",
      }

      files {
        t_httpfiles,
        "Release/src/http/client/http_client_winhttp.cpp",
        "Release/src/pplx/pplxwin.cpp",
      }

      buildoptions {
        "/sdl",
      }

    -- -------------------------------------------------------------
    -- configuration { "windows", "Debug", "x32" }
    -- -------------------------------------------------------------

    -- common configuration settings

    dofile (_BUILD_DIR .. "/static_win_x86_debug.lua")

    -- project specific configuration settings

    -- configuration { "windows", "Debug", "x32" }

    -- -------------------------------------------------------------
    -- configuration { "windows", "Debug", "x64" }
    -- -------------------------------------------------------------

    -- common configuration settings

    dofile (_BUILD_DIR .. "/static_win_x64_debug.lua")

    -- project specific configuration settings

    -- configuration { "windows", "Debug", "x64" }

    -- -------------------------------------------------------------
    -- configuration { "windows", "Release", "x32" }
    -- -------------------------------------------------------------

    -- common configuration settings

    dofile (_BUILD_DIR .. "/static_win_x86_release.lua")

    -- project specific configuration settings

    -- configuration { "windows", "Release", "x32" }

    -- -------------------------------------------------------------
    -- configuration { "windows", "Release", "x64" }
    -- -------------------------------------------------------------

    -- common configuration settings

    dofile (_BUILD_DIR .. "/static_win_x64_release.lua")

    -- project specific configuration settings

    -- configuration { "windows", "Release", "x64" }

    -- -------------------------------------------------------------
  end

  if (_PLATFORM_LINUX) then
    -- -------------------------------------------------------------
    -- configuration { "linux" }
    -- -------------------------------------------------------------

    -- common configuration settings

    dofile (_BUILD_DIR .. "/static_linux.lua")

    -- project specific configuration settings

    configuration { "linux" }

      files {
        t_httpfiles,
        "Release/src/http/client/http_client_asio.cpp",
        "Release/src/http/client/x509_cert_utilities.cpp",
        "Release/src/pplx/pplxlinux.cpp",
        "Release/src/pplx/threadpool.cpp",
      }

      includedirs {
        "../openssl/include",
      }

    -- -------------------------------------------------------------
    -- configuration { "linux", "Debug", "x64" }
    -- -------------------------------------------------------------

    -- common configuration settings

    dofile (_BUILD_DIR .. "/static_linux_x64_debug.lua")

    -- project specific configuration settings

    -- configuration { "linux", "Debug", "x64" }

    -- -------------------------------------------------------------
    -- configuration { "linux", "Release", "x64" }
    -- -------------------------------------------------------------

    -- common configuration settings

    dofile (_BUILD_DIR .. "/static_linux_x64_release.lua")

    -- project specific configuration settings

    -- configuration { "linux", "Release", "x64" }

    -- -------------------------------------------------------------
    -- configuration { "linux", "Debug", "ARM64" }
    -- -------------------------------------------------------------

    -- common configuration settings

    dofile (_BUILD_DIR .. "/static_linux_arm64_debug.lua")

    -- project specific configuration settings

    -- configuration { "linux", "Debug", "ARM64" }

    -- -------------------------------------------------------------
    -- configuration { "linux", "Release", "ARM64" }
    -- -------------------------------------------------------------

    -- common configuration settings

    dofile (_BUILD_DIR .. "/static_linux_arm64_release.lua")

    -- project specific configuration settings

    -- configuration { "linux", "Release", "ARM64" }

    -- -------------------------------------------------------------
  end

  if (_PLATFORM_MACOS) then
    -- -------------------------------------------------------------
    -- configuration { "macosx" }
    -- -------------------------------------------------------------

    -- common configuration settings

    dofile (_BUILD_DIR .. "/static_mac.lua")

    -- project specific configuration settings

    configuration { "macosx" }

      files {
        "Release/src/pplx/pplxapple.cpp",
      }

    -- -------------------------------------------------------------
    -- configuration { "macosx", "Debug", "x64" }
    -- -------------------------------------------------------------

    -- common configuration settings

    dofile (_BUILD_DIR .. "/static_mac_x64_debug.lua")

    -- project specific configuration settings

    -- configuration { "macosx", "Debug", "x64" }

    -- -------------------------------------------------------------
    -- configuration { "macosx", "Release", "x64" }
    -- -------------------------------------------------------------

    -- common configuration settings

    dofile (_BUILD_DIR .. "/static_mac_x64_release.lua")

    -- project specific configuration settings

    -- configuration { "macosx", "Release", "x64" }

    -- -------------------------------------------------------------
  end

  if (_PLATFORM_IOS) then
    -- -------------------------------------------------------------
    -- configuration { "ios*" }
    -- -------------------------------------------------------------

    -- common configuration settings

    dofile (_BUILD_DIR .. "/static_ios.lua")

    -- project specific configuration settings

    configuration { "ios*" }

      files {
        "Release/src/pplx/pplxapple.cpp",
      }

    -- -------------------------------------------------------------
    -- configuration { "ios_arm64_debug" }
    -- -------------------------------------------------------------

    -- common configuration settings

    dofile (_BUILD_DIR .. "/static_ios_arm64_debug.lua")

    -- project specific configuration settings

    -- configuration { "ios_arm64_debug" }

    -- -------------------------------------------------------------
    -- configuration { "ios_arm64_release" }
    -- -------------------------------------------------------------

    -- common configuration settings

    dofile (_BUILD_DIR .. "/static_ios_arm64_release.lua")

    -- project specific configuration settings

    -- configuration { "ios_arm64_release" }

    -- -------------------------------------------------------------
    -- configuration { "ios_sim64_debug" }
    -- -------------------------------------------------------------

    -- common configuration settings

    dofile (_BUILD_DIR .. "/static_ios_sim64_debug.lua")

    -- project specific configuration settings

    -- configuration { "ios_sim64_debug" }

    -- -------------------------------------------------------------
    -- configuration { "ios_sim64_release" }
    -- -------------------------------------------------------------

    -- common configuration settings

    dofile (_BUILD_DIR .. "/static_ios_sim64_release.lua")

    -- project specific configuration settings

    -- configuration { "ios_sim64_release" }

    -- -------------------------------------------------------------
  end

  if (_PLATFORM_ANDROID) then
    -- -------------------------------------------------------------
    -- configuration { "android*" }
    -- -------------------------------------------------------------

    -- common configuration settings

    dofile (_BUILD_DIR .. "/static_android.lua")

    -- project specific configuration settings

    configuration { "android*" }

      files {
        t_httpfiles,
        "Release/src/http/client/http_client_asio.cpp",
        "Release/src/http/client/x509_cert_utilities.cpp",
        "Release/src/pplx/pplxlinux.cpp",
        "Release/src/pplx/threadpool.cpp",
      }

      includedirs {
        "../openssl/include",
      }

    -- -------------------------------------------------------------
    -- configuration { "android_armv7_debug" }
    -- -------------------------------------------------------------

    -- common configuration settings

    dofile (_BUILD_DIR .. "/static_android_armv7_debug.lua")

    -- project specific configuration settings

    -- configuration { "android_armv7_debug" }

    -- -------------------------------------------------------------
    -- configuration { "android_armv7_release" }
    -- -------------------------------------------------------------

    -- common configuration settings

    dofile (_BUILD_DIR .. "/static_android_armv7_release.lua")

    -- project specific configuration settings

    -- configuration { "android_armv7_release" }

    -- -------------------------------------------------------------
    -- configuration { "android_x86_debug" }
    -- -------------------------------------------------------------

    -- common configuration settings

    dofile (_BUILD_DIR .. "/static_android_x86_debug.lua")

    -- project specific configuration settings

    -- configuration { "android_x86_debug" }

    -- -------------------------------------------------------------
    -- configuration { "android_x86_release" }
    -- -------------------------------------------------------------

    -- common configuration settings

    dofile (_BUILD_DIR .. "/static_android_x86_release.lua")

    -- project specific configuration settings

    -- configuration { "android_x86_release" }

    -- -------------------------------------------------------------
    -- configuration { "android_arm64_debug" }
    -- -------------------------------------------------------------

    -- common configuration settings

    dofile (_BUILD_DIR .. "/static_android_arm64_debug.lua")

    -- project specific configuration settings

    -- configuration { "android_arm64_debug" }

    -- -------------------------------------------------------------
    -- configuration { "android_arm64_release" }
    -- -------------------------------------------------------------

    -- common configuration settings

    dofile (_BUILD_DIR .. "/static_android_arm64_release.lua")

    -- project specific configuration settings

    -- configuration { "android_arm64_release" }

    -- -------------------------------------------------------------
    -- configuration { "android_x64_debug" }
    -- -------------------------------------------------------------

    -- common configuration settings

    dofile (_BUILD_DIR .. "/static_android_x64_debug.lua")

    -- project specific configuration settings

    -- configuration { "android_x64_debug" }

    -- -------------------------------------------------------------
    -- configuration { "android_x64_release" }
    -- -------------------------------------------------------------

    -- common configuration settings

    dofile (_BUILD_DIR .. "/static_android_x64_release.lua")

    -- project specific configuration settings

    -- configuration { "android_x64_release" }

    -- -------------------------------------------------------------
  end

  if (_PLATFORM_WINUWP) then
    -- -------------------------------------------------------------
    -- configuration { "windows" }
    -- -------------------------------------------------------------

    -- common configuration settings

    dofile (_BUILD_DIR .. "/static_winuwp.lua")

    -- project specific configuration settings

    configuration { "windows" }

      defines {
        "CPPREST_FORCE_PPLX",
      }

      files {
        t_httpfiles,
        "Release/src/http/client/http_client_winrt.cpp",
        "Release/src/pplx/pplxwin.cpp",
      }

      buildoptions {
        "/sdl",
        "/ZW",
        "/EHsc",
        "/AI\"$(VCIDEInstallDir)vcpackages\"",
        "/AI\"$(WindowsSDK_UnionMetadataPath)\"",
      }

    -- -------------------------------------------------------------
    -- configuration { "winuwp_debug", "x32" }
    -- -------------------------------------------------------------

    -- common configuration settings

    dofile (_BUILD_DIR .. "/static_winuwp_x86_debug.lua")

    -- project specific configuration settings

    -- configuration { "winuwp_debug", "x32" }

    -- -------------------------------------------------------------
    -- configuration { "winuwp_release", "x32" }
    -- -------------------------------------------------------------

    -- common configuration settings

    dofile (_BUILD_DIR .. "/static_winuwp_x86_release.lua")

    -- project specific configuration settings

    -- configuration { "winuwp_release", "x32" }

    -- -------------------------------------------------------------
    -- configuration { "winuwp_debug", "x64" }
    -- -------------------------------------------------------------

    -- common configuration settings

    dofile (_BUILD_DIR .. "/static_winuwp_x64_debug.lua")

    -- project specific configuration settings

    -- configuration { "winuwp_debug", "x64" }

    -- -------------------------------------------------------------
    -- configuration { "winuwp_release", "x64" }
    -- -------------------------------------------------------------

    -- common configuration settings

    dofile (_BUILD_DIR .. "/static_winuwp_x64_release.lua")

    -- project specific configuration settings

    -- configuration { "winuwp_release", "x64" }

    -- -------------------------------------------------------------
    -- configuration { "winuwp_debug", "ARM64" }
    -- -------------------------------------------------------------

    -- common configuration settings

    dofile (_BUILD_DIR .. "/static_winuwp_arm64_debug.lua")

    -- project specific configuration settings

    -- configuration { "winuwp_debug", "ARM64" }

    -- -------------------------------------------------------------
    -- configuration { "winuwp_release", "ARM64" }
    -- -------------------------------------------------------------

    -- common configuration settings

    dofile (_BUILD_DIR .. "/static_winuwp_arm64_release.lua")

    -- project specific configuration settings

    -- configuration { "winuwp_release", "ARM64" }

    -- -------------------------------------------------------------
  end

  if (_IS_QT) then
    -- -------------------------------------------------------------
    -- configuration { "qt" }
    -- -------------------------------------------------------------

    local qt_include_dirs = PROJECT_INCLUDE_DIRS

    -- Add additional QT include dirs
    -- table.insert(qt_include_dirs, <INCLUDE_PATH>)

    createqtfiles(project(), qt_include_dirs)

    -- -------------------------------------------------------------
  end
