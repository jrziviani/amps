My name is {= name =}

Cities:
{% for count in range(0, 2, 1) %}
  Iteration #{= count =}
  {% for subcount in range(0, 4, 4) %}
    Subiteration #{= subcount =}
    {% for city in cities %}
      City: {= city =}
    {% endfor %}
  {% endfor %}
{% endfor %}

Likes:
{% for hobby, like in likes %}
  {% if hobby eq "song" and like eq "gnr" %}
    WOoooow, Guns 'N' Roses!
  {% else %}
    {= hobby =}: {= like =}
  {% endif %}
{% endfor %}

{= cities[0] =}
{= likes["song"] =}
