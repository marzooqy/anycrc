from src import anycrc
import sys

word_size = 64 if sys.maxsize > 2 ** 32 else 32

#The standard test data b'123456789' isn't good enough when testing slice-by-16 because the data length is < 16
test_data = b'abcdefghijklmnopqrstuvwxyz'

for model, params in anycrc.models.items():
    if word_size == 32 and params.width > 32:
        break
        
    crc = anycrc.Model(model)
    crc2 = anycrc.Model(model)
    
    #read all at once
    value = crc.calc(test_data)
    value2 = crc2._calc(test_data)
    
    print(model)
    print(value, value2)
    print()
    
    assert value == value2
    
    #read one char at a time
    crc.reset()
    
    for c in test_data:
        value = crc.calc(c.to_bytes(1, 'little'))
    
    assert value == value2