from src import crcany

test_data = b'123456789'

for model, params in crcany.models.items():
    crc = crcany.Model(model)
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
        value = crc.calc(c.to_bytes(1))
    
    assert value == check