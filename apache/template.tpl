<html>
    <head>
        <meta charset="utf-8">
    </head>

    <body>
        <h2 class="odd">Welcome {= user_data["name"] =}</h2>
        <ul>
            <li>ALL DATA:</ul>
            {% for key, value in user_data %}
                {% if value eq "<null>" %}
                    <li>ops, something wrong here</li>
                {% else %}
                    <li>{= value =}</li>
                {% endif %}
            {% endfor %}
        </ul>
    </body>
</html>
