import cyrc

test_data = b'123456789'

for check_name in dir(cyrc.models):
    if check_name.endswith('_check'):
        model_name = check_name[:-6]
        check = getattr(cyrc.models, check_name)
        
        crc = getattr(cyrc.models, model_name)
        
        #read all at once
        val = crc.init()
        val = crc.update(val, test_data)
        val = crc.finalize(val)
        
        print(model_name)
        print(val, check)
        print()
        
        assert val == check
        
        #read one char at a time
        val = crc.init()
        
        for c in test_data:
            val = crc.update(val, c.to_bytes(1))
        
        assert crc.finalize(val) == check