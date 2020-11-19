set_config("cxxflags", "-fpermissive")
target("coroutine")
    set_kind("static")
    add_files("./coroutine/coroutine.c")

target("pystring")
    set_kind("static")
    add_files("./pystring/pystring.cpp")