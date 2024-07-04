import anycrc
import sys

if len(sys.argv) > 1 and sys.argv[1] == 'models':
    print('Models:')
    for name, model in anycrc.models.items():
        print("'{}': {}".format(name, anycrc.str_model(model)))

    print('\nAliases:')
    for name in anycrc.models:
        aliases = []

        for alias, model_name in anycrc.aliases.items():
            if name == model_name:
                aliases.append(alias)

        if len(aliases) > 0:
            print("'{}': {}".format(name, aliases))

    print()