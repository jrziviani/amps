How to compile the wrapper
==========================

```sh
$ g++ -std=c++17 -I/home/ziviani/amps/include -I/home/ziviani/httpd/www/include -fPIC amps_wrapper.cpp -o wrapper.o -c -g3
```

How to compile the module
=========================

Using static amps library

```
../bin/apxs  -I/home/ziviani/amps/include -c -i mod_cool_framework.c wrapper.o libamps-static.a
```

