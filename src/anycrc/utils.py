def get_hex(value, width):
    str_width = width // 4
    if width % 4 != 0:
        str_width += 1

    return ''.join(['0x{:0', str(str_width), 'x}']).format(value)

def str_model(model):
    return 'width={}, poly={}, init={}, refin={}, refout={}, xorout={}, check={}'.format(model.width, get_hex(model.poly, model.width),
    get_hex(model.init, model.width), model.refin, model.refout, get_hex(model.xorout, model.width), get_hex(model.check, model.width))