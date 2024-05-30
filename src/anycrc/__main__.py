import anycrc
import sys

def get_hex(value, width):
    return '0x{:0widthx}'.replace('width', str(width)).format(value)
    
if len(sys.argv) > 1 and sys.argv[1] == 'models':
    print('Models:')
    for name, model in anycrc.models.items():
        disp_width = model.width // 4
        if model.width % 4 != 0:
            disp_width += 1
            
        print(" '" + name + "'", end=': ')
        print('width={}'.format(model.width), end=', ')
        print('poly={}'.format(get_hex(model.poly, disp_width)), end=', ')
        print('init={}'.format(get_hex(model.init, disp_width)), end=', ')
        print('refin={}'.format(model.refin), end=', ')
        print('refout={}'.format(model.refout), end=', ')
        print('xorout={}'.format(get_hex(model.xorout, disp_width)), end=', ')
        print('check={}'.format(get_hex(model.check, disp_width)))
        
    print('\nAliases:')
    for model in anycrc.models.keys():
        aliases = []
        
        for alias, model_name in anycrc.aliases.items():
            if model == model_name:
                aliases.append(alias)
                
        if len(aliases) > 0:
            print(" '" + model + "'", end=': ')
            print(aliases)
            
    print()