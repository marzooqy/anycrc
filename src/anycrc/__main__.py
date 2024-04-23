import anycrc
import pprint
import sys

if len(sys.argv) > 1 and sys.argv[1] == 'models':
    print('Models:')
    pprint.pprint(anycrc.models, sort_dicts=False)
    print('\nAliases:')
    pprint.pprint(anycrc.aliases, sort_dicts=False)
    print()