from bitarray import bitarray
import anycrc
import sys

if len(sys.argv) > 1:
    if sys.argv[1] == 'models':
        #The standard test data b'123456789' isn't good enough when testing slice-by-16 because the data length is < 16
        test_data = b'123456789'
        test_data2 = b'abcdefghijklmnopqrstuvwxyz'

        for name, model in anycrc.models.items():
            print(name)
            print(anycrc.str_model(model))

            crc = anycrc.Model(name)
            crc2 = anycrc.Model(name)

            #check to see if the result from the byte-by-byte algorithm matches the check value
            value = crc._calc_b(test_data)
            check = model.check
            print('byte-by-byte:  {} {}'.format(anycrc.get_hex(value, model.width), anycrc.get_hex(check, model.width)))
            assert value == check

            #check to see if the result from the slice-by-16 algorithm matches the byte-by-byte value
            value = crc.calc(test_data2)
            value2 = crc2._calc_b(test_data2)

            print('slice-by-16:   {} {}'.format(anycrc.get_hex(value, model.width), anycrc.get_hex(value2, model.width)))
            assert value == value2

            #process the data one byte at a time
            for c in test_data:
                value = crc.update(c.to_bytes(1, 'little'))

            print('update:        {} {}'.format(anycrc.get_hex(value, model.width), anycrc.get_hex(check, model.width)))
            assert value == check
            crc.reset()

            #with length in bits
            if not model.refin:
                bit_data = bitarray()
                bit_data.frombytes(test_data2)
                crc.update_bits(bit_data[:100])
                value = crc.update_bits(bit_data[100:])
                value2 = crc2._calc_b(test_data2)

                print('bits:          {} {}'.format(anycrc.get_hex(value, model.width), anycrc.get_hex(value2, model.width)))
                assert value == value2
                crc.reset()

            #combine
            value = crc.calc(b'12345')
            value2 = crc.calc(b'6789')
            crc.set(value)
            value3 = crc.combine(value2, len(b'6789'))
            print('combine:       {} {}'.format(anycrc.get_hex(value3, model.width), anycrc.get_hex(check, model.width)))
            assert value3 == check
            crc.reset()

            #combine bits
            if not model.refin:
                bit_data = bitarray()
                bit_data.frombytes(test_data)
                value = crc.calc_bits(bit_data[:36])
                value2 = crc.calc_bits(bit_data[36:])
                crc.set(value)
                value3 = crc.combine_bits(value2, len(bit_data[36:]))
                print('combine bits:  {} {}'.format(anycrc.get_hex(value3, model.width), anycrc.get_hex(check, model.width)))
                assert value3 == check
                crc.reset()

            print()

    #check that all of the aliases are valid
    elif sys.argv[1] == 'aliases':
        for alias, name in anycrc.aliases.items():
            print(alias, end=' ')
            assert name in anycrc.models
            print('OK')

        print()

elif len(sys.argv) == 1:
    print('Test name is not specified')
