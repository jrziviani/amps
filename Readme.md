Volt - Simple Template Engine for C++
=====================================

Introduction
------------

**IMPORTANT**: It's not production ready, it's not even finished yet. So don't use it for anything important.

Volt offers a simple and small text-based template language for general purposes. It's inspired by [Jinja2](http://jinja.pocoo.org) - but light-years away from the quality achieved by Jinja2.

What it does
------------

Given a template file and an user data, Volt will use that user data to create a result file based on the template file.

A basic template looks like:

```bash
% cat template.tpl```

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
</html>```

```bash
% cat include.tpl```

```html
<p>Hello, {= name =}. From include.tpl</p>```

The C++ code is:
```cpp
volt::user_map ht {
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
</html>```

Building
--------

There is a build script provided. Running:

```bash
% ./build --release```

Will create a folder called bin/release with:

```include.tpl  libvolt.so  template.tpl  volt_sample```

The file **libvolt.so** is the important thing here, **volt_sample** is an example on how to use that lib. The sample will run with:

```bash
% ./volt_sample template.tpl```

It's also possible to create the same objects with debug symbols included:

```bash
% ./build --debug```

Or building a static library version for both, cleaning the current tree:

```bash
% ./build --clear --debug --release --static```

Testing
-------

```
% ./build --test```
