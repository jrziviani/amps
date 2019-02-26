How to compile the wrapper
==========================

```sh
$ httpd/www/build/libtool --mode=compile g++ -std=c++17 -I/home/ziviani/amps/include -I/home/ziviani/httpd/www/include -fPIC amps_wrapper.cpp -lm -o wrapper.lo -c -g3
```

How to compile the module
=========================

Using static amps library

```
../bin/apxs  -I/home/ziviani/amps/include -c -i mod_cool_framework.c wrapper.o libamps-static.a
$ httpd/www/bin/apxs -I/home/ziviani/ziviani/amps/include -c -i mod_cool_framework.c wrapper.lo libamps.so
```
