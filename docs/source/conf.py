#!/usr/bin/env python3
project = 'YosysHQ MCY'
author = 'YosysHQ GmbH'
copyright ='2021 YosysHQ GmbH'

templates_path = ['_templates']

# select HTML theme
html_theme = "furo"

# These folders are copied to the documentation's HTML output
html_static_path = ['../static', '../images']

html_logo = '../static/logo.png'
html_favicon = '../static/favico.png'
html_css_files = ['custom.css']

# code blocks style 
pygments_style = 'colorful'
highlight_language = 'systemverilog'

html_theme_options = {
    "sidebar_hide_name": True,

    "light_css_variables": {
        "color-brand-primary": "#d6368f",
        "color-brand-content": "#4b72b8",
        "color-api-name": "#8857a3",
        "color-api-pre-name": "#4b72b8",
        "color-link": "#8857a3",
    },

    "dark_css_variables": {
        "color-brand-primary": "#e488bb",
        "color-brand-content": "#98bdff",
        "color-api-name": "#8857a3",
        "color-api-pre-name": "#4b72b8",
        "color-link": "#be95d5",
    },
}

extensions = ['sphinx.ext.autosectionlabel']
