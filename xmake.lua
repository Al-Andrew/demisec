add_rules("mode.debug", "mode.release")
set_languages("c17")


target("FieldKit")
    set_kind("static")
    add_files("fieldkit/src/*.c")
    add_includedirs("fieldkit/include/", {public = true});


target("demisec")
    set_kind("binary")
    add_files("demisec/src/*.c")
    add_includedirs("demisec/include/")
    add_deps("FieldKit")
