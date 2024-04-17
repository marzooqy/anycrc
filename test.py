from src import anycrc

test_data = b'123456789'

for model, params in anycrc.models.items():
    crc = anycrc.Model(model)
    check = params.check
    
    #read all at once
    value = crc.calc(test_data)
    
    print(model)
    print(value, check)
    print()
    
    assert value == check
    
    #read one char at a time
    crc.reset()
    
    for c in test_data:
        value = crc.calc(c.to_bytes(1, 'little'))
    
    assert value == check
