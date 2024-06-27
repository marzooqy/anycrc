import anycrc
import sys

#The standard test data b'123456789' isn't good enough when testing slice-by-16 because the data length is < 16
test_data = b'123456789'
test_data2 = b'abcdefghijklmnopqrstuvwxyz'

for name, model in anycrc.models.items():
    if model.width > anycrc.word_bits * 2:
        break

    print(name)
    print(anycrc.str_model(model))

    crc = anycrc.Model(name)
    crc2 = anycrc.Model(name)

    #read all at once
    value = crc.calc(test_data)
    check = model.check

    print('1 byte whole:     {} {}'.format(anycrc.get_hex(value, model.width), anycrc.get_hex(check, model.width)))
    assert value == check

    #read one char at a time
    for c in test_data:
        value = crc.update(c.to_bytes(1, 'little'))

    print('1 byte partial:   {} {}'.format(anycrc.get_hex(value, model.width), anycrc.get_hex(check, model.width)))
    assert value == check
    crc.reset()

    if model.width <= anycrc.word_bits:
        #read all at once
        value = crc.calc(test_data2)
        value2 = crc2._calc_b(test_data2)

        print('16 bytes whole:   {} {}'.format(anycrc.get_hex(value, model.width), anycrc.get_hex(value2, model.width)))
        assert value == value2

        #read one char at a time
        for c in test_data2:
            value = crc.update(c.to_bytes(1, 'little'))

        print('16 bytes partial: {} {}'.format(anycrc.get_hex(value, model.width), anycrc.get_hex(value2, model.width)))
        assert value == value2

    print()

#check that all of the aliases are valid
print('Aliases:')
for alias, name in anycrc.aliases.items():
    assert name in anycrc.models
    print(alias + ' OK')

print()