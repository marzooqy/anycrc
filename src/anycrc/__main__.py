import anycrc
import pprint
import sys

if len(sys.argv) > 1 and sys.argv[1] == 'models':
    pprint.pprint(anycrc.models, sort_dicts=False)