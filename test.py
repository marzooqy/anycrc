from src import anycrc
import sys

word_size = 64 if sys.maxsize > 2 ** 32 else 32

#The standard test data b'123456789' isn't good enough when testing slice-by-16 because the data length is < 16
test_data = b'123456789'
test_data2 = b'abcdefghijklmnopqrstuvwxyz'

for model, params in anycrc.models.items():
    if word_size == 32 and params.width > 32:
        break
        
    if params.width % 4 == 0:
        disp_width = params.width // 4
    else:
        disp_width = params.width // 4 + 1
        
    fmt_str = '0x{:0widthx} 0x{:0widthx}'.replace('width', str(disp_width))
    
    print(model)
    
    crc = anycrc.Model(model)   
    crc2 = anycrc.Model(model)
    
    #read all at once
    value = crc.calc(test_data)
    check = params.check
    
    print('1 byte whole:     ' + fmt_str.format(value, check))
    assert value == check
    crc.reset()
    
    #read one char at a time
    for c in test_data:
        value = crc.calc(c.to_bytes(1, 'little'))
    
    print('1 byte partial:   ' + fmt_str.format(value, check))
    assert value == check
    crc.reset()
    
    #read all at once
    value = crc.calc(test_data2)
    value2 = crc2._calc_b(test_data2)

    print('16 bytes whole:   ' + fmt_str.format(value, value2))
    assert value == value2
    crc.reset()
    
    #read one char at a time
    for c in test_data2:
        value = crc.calc(c.to_bytes(1, 'little'))

    print('16 bytes partial: ' + fmt_str.format(value, value2))
    assert value == value2
    
    crc.reset()
    value = crc._calc_p(test_data2)
    print('parallel whole:   ' + fmt_str.format(value, value2))
    assert value == value2
    print()
    
#check that all of the aliases are valid
print('Aliases:')
for alias, name in anycrc.aliases.items():
    assert name in anycrc.models
    print(alias + ' OK')
    
print()