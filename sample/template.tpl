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
