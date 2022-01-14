set_project("Demisec")
set_languages("c17")
add_rules("mode.debug", "mode.release")

add_requires("libgcrypt")


target("FieldKit")
    set_kind("static")
    add_files("fieldkit/src/*.c")
    add_includedirs("fieldkit/include/", {public = true});
    add_packages("libgcrypt", {public = true});

target("demo")
    set_kind("binary")
    add_files("scratchpad.c")

target("demo-aes")
    set_kind("binary")
    add_files("aes-gcrypt.c")
    add_packages("libgcrypt")

target("demo-rsa")
    set_kind("binary")
    add_files("rsa-gcrypt.c")
    add_packages("libgcrypt")
    add_deps("FieldKit")

target("demisec_sv")
    set_kind("binary")
    add_files("demisec/server/*.c")
    add_deps("FieldKit")

target("demisec_cl")
    set_kind("binary")
    add_files("demisec/client/*.c")
    add_deps("FieldKit")
