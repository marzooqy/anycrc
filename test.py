from bitarray import bitarray
import anycrc

failed = False

test_data = b'123456789'
test_data2 = bytes(i for i in range(256))

for name, model in anycrc.models.items():
    print(name)
    print(anycrc.str_model(model))

    crc = anycrc.Model(name)

    #check to see if the result from the byte-by-byte algorithm matches the check value
    value = crc._calc_b(test_data)
    check = model.check
    print('byte-by-byte:  {} {}'.format(anycrc.get_hex(value, model.width), anycrc.get_hex(check, model.width)))
    if value != check:
        failed = True

    #check to see if the result from the clmul algorithm matches the byte-by-byte value
    value = crc.calc(test_data2)
    value2 = crc._calc_b(test_data2)

    print('clmul:         {} {}'.format(anycrc.get_hex(value, model.width), anycrc.get_hex(value2, model.width)))
    if value != value2:
        failed = True

    #process the data one byte at a time
    value = None
    for c in test_data:
        value = crc.calc(c.to_bytes(1, 'little'), value)

    print('update:        {} {}'.format(anycrc.get_hex(value, model.width), anycrc.get_hex(check, model.width)))
    if value != check:
        failed = True

    #with length in bits
    if model.refin:
        bit_data = bitarray(endian='little')
    else:
        bit_data = bitarray()

    bit_data.frombytes(test_data2)
    value = crc.calc_bits(bit_data[:100])
    value = crc.calc_bits(bit_data[100:], value)
    value2 = crc._calc_b(test_data2)

    print('bits:          {} {}'.format(anycrc.get_hex(value, model.width), anycrc.get_hex(value2, model.width)))
    if value != value2:
        failed = True

    #combine
    value = crc.calc(b'12345')
    value2 = crc.calc(b'6789')
    value3 = crc.combine(value, value2, len(b'6789'))
    print('combine:       {} {}'.format(anycrc.get_hex(value3, model.width), anycrc.get_hex(check, model.width)))
    if value3 != check:
        failed = True

    #combine bits
    if model.refin:
        bit_data = bitarray(endian='little')
    else:
        bit_data = bitarray()

    bit_data.frombytes(test_data)
    value = crc.calc_bits(bit_data[:36])
    value2 = crc.calc_bits(bit_data[36:])
    value3 = crc.combine_bits(value, value2, len(bit_data[36:]))
    print('combine bits:  {} {}'.format(anycrc.get_hex(value3, model.width), anycrc.get_hex(check, model.width)))
    if value3 != check:
        failed = True

    print()

#check that all of the aliases are valid
for alias, name in anycrc.aliases.items():
    print(alias, end=' ')
    if name in anycrc.models:
        print('OK')
    else:
        failed = True
        print('FAIL')

print()

if failed:
    raise Exception('Test failed')