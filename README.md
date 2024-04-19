# BGE-File
A somewhat nice and easy (binary-level) file reader and writer utility in a single header for C++.
Shamelessly ripped out of a personal game engine project and made public
because it do be useful sometimes in other projects.
<br/>
Feel free to use, but remember to at least credit me (GAMINGNOOBdev)
as the owner/creator of this file.

## BgeFile.hpp
This is just a handy little file reader, nothing else.

## BgeConfig.hpp
This is a config file reader that can read TOML-like files, but has the gamingnoob twist
in it and can only hold string, integer, float and boolean values. Oh yeah and it has a
"section" system.

Example of a config file:
```
[General]
Setting0 = false
...

[General.Editor]
Setting0 = true
...

[ENDSECTION] // this ends the current section entirely
             // so we switch to the global address space

GlobalSetting0 = 10
...
```

The interpreted version:
```
Sections:
[
    Section{
        Name: "General"
        Properties:
        [
            Setting0: false,
            ...
        ]
        SubSections:
        [
            Section{
                Name: "Editor"
                Properties:
                [
                    Setting0 = true,
                    ...
                ]
            }
        ]
    }
]

Properties:
[
    GlobalSetting0 = 10,
    ...
]
```