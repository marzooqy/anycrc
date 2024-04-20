import anycrc
import pprint
import sys

if __name__ == '__main__':
    if sys.argv[1] == 'models':
        pprint.pprint(anycrc.models, sort_dicts=False)