#Download the latest models from the CRC RevEng website

from urllib.request import urlopen
from bs4 import BeautifulSoup

url = 'https://reveng.sourceforge.io/crc-catalogue/all.htm'

with urlopen(url) as response:
    page = response.read()

soup = BeautifulSoup(page, 'html.parser')
tags = soup.select('h3 + p > code, h3 + p + ul > li > a > strong')

crcs = {}
aliases = {}

current_crc = ''
current_width = 0

for tag in tags:
    if tag.name == 'code':
        name = tag.text.partition('name="')[2].rstrip('"')
        name = name.replace('-', '', 1).replace('/', '-')

        params = tag.text.partition('  residue=')[0]
        params = params.replace('  ', ', ').replace('true', 'True').replace('false', 'False')

        width_start = tag.text.find('width=') + len('width=')
        width_end = tag.text.find('  ', width_start)
        width = int(params[width_start:width_end])

        current_crc = name
        current_width = width

        if width <= 64:
            crcs[name] = params

    elif tag.name == 'strong' and current_width <= 64:
        name = tag.text.replace('/', '-')
        if name.startswith('CRC-') and name[4].isnumeric():
            name = name.replace('-', '', 1)

        aliases[name] = current_crc

with open('crcs.txt', 'w') as file:
    file.write('models = {\n')
    for model, params in crcs.items():
        file.write(f"    '{model}': model({params}),\n")

    file.write('}\n\n')
    file.write('aliases = {\n')

    for alias, model in aliases.items():
        file.write(f"    '{alias}': '{model}',\n")

    file.write('}')
