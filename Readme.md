Amps - Simple Template Engine for C++ 
=====================================
[![Codacy Badge](https://api.codacy.com/project/badge/Grade/f3b1aa0870124e58901854f8fbcbc87b)](https://www.codacy.com/app/jrziviani/amps?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=jrziviani/amps&amp;utm_campaign=Badge_Grade) [![Build Status](https://travis-ci.com/jrziviani/amps.svg?branch=master)](https://travis-ci.com/jrziviani/amps) [![codecov](https://codecov.io/gh/jrziviani/amps/branch/master/graph/badge.svg)](https://codecov.io/gh/jrziviani/amps) [![Coverage Status](https://coveralls.io/repos/github/jrziviani/amps/badge.svg)](https://coveralls.io/github/jrziviani/amps)

Index
-----

* [Introduction](#Introduction)
* [What it does](#What-it-does)
* [Building](#Building)
* [Testing](#Testing)

Introduction
------------

**IMPORTANT**: It's not even finished yet, so don't use it for anything important.

Amps offers a simple and small text-based template language for general purposes. It's inspired by [Jinja2](http://jinja.pocoo.org) - but light-years away from the quality achieved by Jinja2.

What it does
------------

Given a template file and an user data, Amps will use that user data to create a result file based on the template file.

A basic template looks like:

```shell
% cat template.tpl
```

```html
<html>
    <head>
        <meta charset="utf-8">
        <style>
            .odd { background-color: #ccc }
            .even { background-color: #fcfcfc }
        </style>
    </head>

    <body>
        {% insert "include.tpl" %}

        <p>List of cities:</p>
        <ul>
        {% for city in cities %}
            <li>{= city =}</li>
        {% endfor %}
        </ul>
        <p>The city number 4 (zero-indexed) is {= cities[3] =}</p>

        {% insert "include.tpl" %}

        <p>Couting from -5 to +5:</p>
        <ul>
        {% for number in range(-5, 6, 1) %}
          {% if number % 2 eq 0 %}
            <li class="even">{= number =}</li>
          {% else %}
            <li class="odd">{= number =}</li>
          {% endif %}
        {% endfor %}
        </ul>

        {% insert "include.tpl" %}

        <p>Songs:</p>
        <ul>
        {% for band, song in songs %}
            <li>{= band =} : {= song =}</li>
        {% endfor %}
        </ul>
        <p>Aerosmith&acute; song: {= songs["aerosmith"] =}.</p>
    </body>
</html>
```

```shell
% cat include.tpl
```

```html
<p>Hello, {= name =}. From include.tpl</p>
```

The C++ code is:
```cpp
amps::user_map ht {
              {"name", "Jose"},
              {"cities", vector<string>{
                     "Sao Paulo",
                     "Paris",
                     "NYC",
                     "London",
                     "Lisbon"}},
              {"songs", unordered_map<string, string>{
                    {"guns and roses", "patience"},
                    {"aerosmith", "crazy"},
                    {"led zeppelin", "immigrant song"},
                    {"pink floyd", "high hopes"}}},
};

engine.compile(ht);
```

And expected result is:

```html
<html>
    <head>
        <meta charset="utf-8">
        <style>
            .odd  { background-color: #ccc }
            .even  { background-color: #fcfcfc }
        </style>
    </head>

    <body>
<p>Hello, Jose. From include.tpl</p>

        <p>List of cities:</p>
        <ul>
            <li>Sao Paulo</li>
            <li>Paris</li>
            <li>NYC</li>
            <li>London</li>
            <li>Lisbon</li>
        </ul>
        <p>The city number 4 (zero-indexed) is London</p>

<p>Hello, Jose. From include.tpl</p>

        <p>Couting from -5 to +5:</p>
        <ul>
            <li class="odd">-5</li>
            <li class="even">-4</li>
            <li class="odd">-3</li>
            <li class="even">-2</li>
            <li class="odd">-1</li>
            <li class="even">0</li>
            <li class="odd">1</li>
            <li class="even">2</li>
            <li class="odd">3</li>
            <li class="even">4</li>
            <li class="odd">5</li>
        </ul>

<p>Hello, Jose. From include.tpl</p>

        <p>Songs:</p>
        <ul>
            <li>guns and roses : patience</li>
            <li>aerosmith : crazy</li>
            <li>led zeppelin : immigrant song</li>
            <li>pink floyd : high hopes</li>
        </ul>
        <p>Aerosmith&acute; song: crazy.</p>
    </body>
</html>
```

Building
--------

There is a build script provided. Running:

```shell
% ./build --release
```

Will create a folder called bin/release with:

```shell
% ls bin/release
include.tpl  libamps.so  template.tpl  amps_sample
```

The file **libamps.so** is the important thing here, **amps_sample** is an example on how to use that lib. The sample will run with:

```shell
% ./amps_sample template.tpl
```

It's also possible to create the same objects with debug symbols included:

```shell
% ./build --debug
```

Or building a static library version for both, cleaning the current tree:

```shell
% ./build --clear --debug --release --static
```

Testing
-------

```
% ./build --test
```
