from conans import ConanFile


class CpprestConan(ConanFile):
    name = "cpprest"
    version = "2.10.13"
    url = "https://github.com/Esri/cpprestsdk/tree/runtimecore"
    license = "https://github.com/Esri/cpprestsdk/blob/runtimecore/license.txt"
    description = "The C++ REST SDK is a Microsoft project for cloud-based client-server communication in native code using a modern asynchronous C++ API design. This project aims to help C++ developers connect to and interact with services."

    # RTC specific triple
    settings = "platform_architecture_target"

    def package(self):
        base = self.source_folder + "/"
        relative = "3rdparty/cpprestsdk/"

        # headers
        self.copy("*.h*", src=base + "Release/include/cpprest", dst=relative + "Release/include/cpprest")
        self.copy("*.dat", src=base + "Release/include/cpprest", dst=relative + "Release/include/cpprest")
        self.copy("*.h*", src=base + "Release/include/pplx", dst=relative + "Release/include/pplx")

        # libraries
        output = "output/" + str(self.settings.platform_architecture_target) + "/staticlib"
        self.copy("*" + self.name + "*", src=base + "../../" + output, dst=output)
