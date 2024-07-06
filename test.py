import anycrc

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

    #check to see if the result from the byte-by-byte algorithm matches the check value
    value = crc._calc_b(test_data)
    check = model.check

    assert value == check

    #Test 1
    value = crc.calc(test_data2)
    value2 = crc2._calc_b(test_data2)

    print('calc test:    {} {}'.format(anycrc.get_hex(value, model.width), anycrc.get_hex(value2, model.width)))
    assert value == value2

    #Test 2
    #process the data one byte at a time
    for c in test_data:
        value = crc.update(c.to_bytes(1, 'little'))

    print('update test:  {} {}'.format(anycrc.get_hex(value, model.width), anycrc.get_hex(check, model.width)))
    assert value == check
    crc.reset()

    print()

#check that all of the aliases are valid
print('Aliases:')
for alias, name in anycrc.aliases.items():
    print(alias, end=' ')
    assert name in anycrc.models
    print('OK')

print()