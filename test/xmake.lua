includes("third-party")
add_requires("gtest", {system = false})


target("coroutine_test")
    set_default(false)
    set_kind("binary")
    add_files("coroutine_test.cc")
    add_deps("coroutine")

target("socket_test")
    set_default(false)
    on_load(function (target)
        target:add(find_packages("glog"))
    end)
    set_kind("binary")
    add_files("socket_test.cc")
    add_packages("gtest")
    add_deps("libtinyweb")