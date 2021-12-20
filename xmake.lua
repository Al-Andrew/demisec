set_project("Demisec")
set_languages("c17")
add_rules("mode.debug", "mode.release")

add_requires("libgcrypt")


target("FieldKit")
    set_kind("static")
    add_files("fieldkit/src/*.c")
    add_includedirs("fieldkit/include/", {public = true});


target("demisec_sv")
    set_kind("binary")
    add_files("demisec/server/*.c")
    add_deps("FieldKit")
    add_packages("libgcrypt")

target("demisec_cl")
    set_kind("binary")
    add_files("demisec/client/*.c")
    add_deps("FieldKit")
    add_packages("libgcrypt")
