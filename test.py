from src import anycrc

#The standard test data b'123456789' doesn't when testing slice-by-16 because data length < 16
test_data = b'abcdefghijklmnopqrstuvwxyz'

for model, params in anycrc.models.items():
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