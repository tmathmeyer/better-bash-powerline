#PSG (PS1 Generator) -- a better powerline for bash

PSG provides a means of providing structured colors to
bash prompts so that properly blended powerlines work.

PSG reads a configuration from stdin in the form of a
json strucure like the following:
```json
{
    "style": {
        "powerline": "minimal"
    },
    "elements": [
        {
            "name" : "<a name in quotes>",
            "forground": "<256 color terminal color>",
            "background": "<256 color terminal color>",
            "default": "<any quoted text>", //optional
            "options" : {
                "match" : {
                    "from": "/",
                    "to": "\\"
                }
            }
        }
        ...
    ]
}
```
The "style" and "elements" sections are necessary, any other
sections may be ignored however. Valid JSON syntax is a MUST.

Each object in the "elements" array consists of between 3 to
5 properties. The "name", "forground", and "background"
properties are required. The "forground" and "background"
properties are each given an 8bit color code. These codes and
their corresponding colors can be shown by running the 'colors'
binary built by 'make'.

The "default" property is not required, but any text provided
in this property will be shown if none is provided on the
command line.

The "options" property is currently only used for replacing
single characters within each element's displayed text. In
the example provided, all forward slashes are replaced with
a backslash (unicode characters are valid replacements).

If an element has no text associated with it at runtime, it
will not be displayed. Text becomes associated with an
element either through the default property, or through
assignment on the command line, as follows:

```sh
cat ~/.psg.json | psg user="$USER" path="${PWD/$HOME/\~}"
```

This will assign the contents of the $USER variable to the
element named 'user' and the contents of the $PWD variable
to the element named 'path'.

The powerline property defined the type of powerline status
to generate. The options are "minimal" and "full". Example
screenshots are provided:

![minimal](https://github.com/tmathmeyer/better-bash-powerline/blob/master/screenshots/min.png)
![full](https://github.com/tmathmeyer/better-bash-powerline/blob/master/screenshots/full.png)
