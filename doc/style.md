Style
=====

C files
-------

noexpandtab
shiftwidth=8
softtabstop=8

Use egyptian-style functions:

    int
    func(int blah) {
            ...
    }

Don't indent case statements, it should be at the same level as the switch:

    switch (ch) {
    case '1':
            ...
    case '2':
            ...
    default:
            break;
    }

CMake files
-----------

expandtab
shiftwidth=4
softtabstop=4

