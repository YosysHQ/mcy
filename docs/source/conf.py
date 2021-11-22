#!/usr/bin/env python3
project = 'YosysHQ MCY'
author = 'YosysHQ GmbH'
copyright ='2021 YosysHQ GmbH'

# select HTML theme
html_theme = 'press'
html_logo = '../images/logo.png'
html_sidebars = {'**': ['util/searchbox.html', 'util/sidetoc.html']}

# These folders are copied to the documentation's HTML output
html_static_path = ['_static', "../images"]

# code blocks style 
pygments_style = 'colorful'
highlight_language = 'systemverilog'

html_theme_options = {
    'external_links' : [
        ('YosysHQ Docs', 'https://yosyshq.readthedocs.io'),
        ('Blog', 'https://blog.yosyshq.com'),
        ('Website', 'https://www.yosyshq.com'),
    ],
}
