import sys
import array
import struct
import types
import exceptions
import datetime

MAX_ITEM_COUNT = 100000
MAX_KEY_LEN = 255

def anyvalue_decode(s):
	buffer = array.array('B')
	buffer.fromstring(s)
	return _anyvalue_decode_arry(buffer)

def anyvalue_encode(obj):

	buffer = array.array('B')
	_anyvalue_encode_array(obj,buffer)
	return buffer.tostring()

def _anyvalue_encode_array(obj,buffer):

	t = type(obj)
	if t==types.DictType:
		_encode_dict(buffer, obj)
	elif t==types.ListType or t==types.TupleType:
		_encode_list(buffer, obj)
	else:
		raise exceptions.TypeError,obj


def _anyvalue_decode_arry(in_arr):

	if not isinstance(in_arr, array.array):
		raise exceptions.TypeError,'must use array type'

	start_index = 0
	end_index = len(in_arr)
	if end_index<6:
		raise exceptions.ValueError,'decode error:len=%d, too short' % end_index
	if in_arr[start_index]==0x08:  #map
		start_index += 1
		value = {}
		_decode_dict(in_arr, start_index, end_index, value)
		return value
	elif in_arr[start_index]==0x07:  #array
		start_index += 1
		value = []
		_decode_list(in_arr, start_index, end_index, value)
		return value
	else:
		raise exceptions.ValueError,'decode error at index %d' % start_index
	return None

def _decode_normal_type(in_arr, start_index, end_index):

	value_type = in_arr[start_index]
	start_index += 1
	#print 'type=%d' % value_type

	if 0 == value_type:
		return (in_arr[start_index], start_index+1)

	elif 1 == value_type:
		if end_index-start_index<2:
			raise exceptions.ValueError, 'decode error at index %d:reach to end' % (start_index)
		return (in_arr[start_index]<<8 | in_arr[start_index+1], start_index+2)

	elif 2 == value_type:
		if end_index-start_index<4:
			raise exceptions.ValueError, 'decode error at index %d:reach to end' % 	(start_index)
		return (in_arr[start_index]<<24 | in_arr[start_index+1]<<16 | in_arr[start_index+2]<<8 | in_arr[start_index+3],	start_index+4)

	elif 3 == value_type:
		if end_index-start_index<8:
			raise exceptions.ValueError, 'decode error at index %d:reach to end' % 	(start_index)
		(long_long_value,) = struct.unpack('!Q', in_arr[start_index:start_index+8].tostring())
		return (long_long_value, start_index+8)

	elif 4 == value_type:
		str_len = in_arr[start_index]
		start_index += 1
		if end_index-start_index<str_len:
			raise exceptions.ValueError, 'decode error at index %d:reach to end' % (start_index)
		return (in_arr[start_index:start_index+str_len].tostring(), start_index + str_len)

	elif 5 == value_type:
		str_len = in_arr[start_index] << 8 | in_arr[start_index+1]
		start_index += 2
		if end_index-start_index<str_len:
			raise exceptions.ValueError, 'decode error at index %d:reach to end' % (start_index)
		return (in_arr[start_index:start_index+str_len].tostring(), start_index + str_len)

	elif 6 == value_type:
		str_len = in_arr[start_index]<<24 | in_arr[start_index+1]<<16 | in_arr[start_index+2]<<8 | in_arr[start_index+3]
		start_index += 4
		if end_index-start_index<str_len:
			raise exceptions.ValueError, 'decode error at index %d:reach to end' % (start_index)
		return (in_arr[start_index:start_index+str_len].tostring(), start_index + str_len)

	elif 10 == value_type:
		if end_index-start_index < 8:
			raise exceptions.ValueError, 'decode error at index %d:reach to end' % 	(start_index)
		(float_value,) = struct.unpack('!d', in_arr[start_index:start_index+8].tostring())
		return (float_value, start_index+8)

	elif 11 == value_type:
		if end_index-start_index<1:
			raise exceptions.ValueError, 'decode error at index %d:reach to end' % (start_index)
		bool_value = in_arr[start_index]
		if bool_value == 1:
			return (True, start_index+1)
		else:
			return (False, start_index+1)

	elif 12 == value_type:
		return (None,start_index)

	elif 13 == value_type:
		(char_value,) = struct.unpack('!b',in_arr[start_index:start_index+1].tostring())
		return (char_value, start_index+1)

	elif 14 == value_type:
		if end_index-start_index<2:
			raise exceptions.ValueError, 'decode error at index %d:reach to end' % (start_index)
 		(short_value,) = struct.unpack('!h',in_arr[start_index:start_index+2].tostring())
		return (short_value, start_index+2)

	elif 15 == value_type:
		if end_index-start_index<4:
			raise exceptions.ValueError, 'decode error at index %d:reach to end' % 	(start_index)
 		(int_value,) = struct.unpack('!i',in_arr[start_index:start_index+4].tostring())
		return (int_value, start_index+4)

  	elif 16 == value_type:
		if end_index-start_index<8:
			raise exceptions.ValueError, 'decode error at index %d:reach to end' % 	(start_index)
		(long_long_value,) = struct.unpack('!q', in_arr[start_index:start_index+8].tostring())
		return (long_long_value, start_index+8)
	else:
		raise exceptions.ValueError, 'decode error at index %d:type is %d, error' % (start_index, value_type)

def _decode_dict(in_arr, start_index, end_index, dict):
	global MAX_ITEM_COUNT
	global MAX_KEY_LEN

	if end_index-start_index<4:
		raise exceptions.ValueError,'decode error at index %d' % start_index

	str_dict_item_count = in_arr[start_index:start_index+4].tostring()
	(dict_item_count,) = struct.unpack('!I', str_dict_item_count)

	if dict_item_count>MAX_ITEM_COUNT:
		raise exceptions.ValueError, 'decode error at index %d:item count is %d, too large' % (start_index, dict_item_count)

	start_index += 4
	if end_index-start_index==0:
		return
	for i in range(dict_item_count):
		#print 'index:%d, start:%d' % (i,start_index)
		if end_index-start_index<3:
			raise exceptions.ValueError, 'decode error at index %d:reach to end' % 	(start_index)

		key_len = in_arr[start_index]
		if key_len<=0 or key_len>MAX_KEY_LEN:
			raise exceptions.ValueError, 'decode error at index %d:key len is %u, too large' % 	(start_index, key_len)

		start_index += 1
		if end_index-start_index<key_len+1:
			raise exceptions.ValueError, 'decode error at index %d:reach to end' % 	(start_index)

		str_key = in_arr[start_index:start_index+key_len].tostring()

		start_index += key_len;
		value_type = in_arr[start_index]

		if 7 == value_type:
			temp_arr = []
			start_index = _decode_list(in_arr, start_index+1, end_index, temp_arr)
			dict[str_key] = temp_arr

		elif 8 == value_type:
			temp_dict = {}
			start_index = _decode_dict(in_arr, start_index+1, end_index, temp_dict)
			dict[str_key] = temp_dict
		else:
			(dict[str_key], start_index) = _decode_normal_type(in_arr, start_index, end_index)

	return start_index

def _decode_list(in_arr, start_index, end_index, temp_list):

	global MAX_ITEM_COUNT
	if end_index-start_index<4:
		raise exceptions.ValueError,'decode array error at index %d' % start_index

	(list_item_count,) = struct.unpack('!I', in_arr[start_index:start_index+4].tostring())
	if list_item_count>MAX_ITEM_COUNT:
		raise exceptions.ValueError, 'decode array error at index %d:item count is %u, too large' % (start_index, dict_item_count)

	start_index += 4
	if end_index-start_index==0:
		return
	for i in range(list_item_count):
		if end_index-start_index < 2:
			raise exceptions.ValueError, 'decode array error at index %d:reach to end' % (start_index)

		value_type = in_arr[start_index]

		if 7==value_type:
			temp_arr = []
			start_index = _decode_list(in_arr, start_index+1, end_index, temp_arr)
			temp_list.append(temp_arr)
		elif 8==value_type:
			temp_dict = {}
			start_index = _decode_dict(in_arr, start_index+1, end_index, temp_dict)
			temp_list.append(temp_dict)
		else:
			(temp_value, start_index) = _decode_normal_type(in_arr, start_index, end_index)
			temp_list.append(temp_value)
	return start_index

def _encode_normal_type(arr_out, value):

	item_type = type(value)
	if item_type==types.BooleanType:
		arr_out.append(11)
		arr_out.append(1 if value else 0)
	elif item_type==types.IntType or item_type==types.LongType:

		if value >=0 :
			if value>=0 and value<=0xff:
				arr_out.append(0)
				arr_out.append(value)
			elif value>0xff and value<=0xffff:
				arr_out.append(1)
				arr_out.fromstring(struct.pack('!H', value))
			elif value>0xffff and value<=0xffffffff:
				arr_out.append(2)
				arr_out.fromstring(struct.pack('!I', value))
  			else:
				arr_out.append(3)
				arr_out.fromstring(struct.pack('!Q', value))
		else:
			if value > -129 and value < 0:
				arr_out.append(13)
				arr_out.fromstring(struct.pack('!b', value))
			elif value>-32769 and value<=-129:
				arr_out.append(14)
				arr_out.fromstring(struct.pack('!h', value))
	 		elif value>-2147483649 and value<=-32769:
				arr_out.append(15)
				arr_out.fromstring(struct.pack('!i', value))
			else:
				arr_out.append(16)
				arr_out.fromstring(struct.pack('!q', value))

	elif item_type==types.FloatType:
		arr_out.append(10)
		arr_out.fromstring(struct.pack('!d', value))

	elif item_type==types.StringType or item_type==types.UnicodeType:
		if item_type==types.UnicodeType:
			value = value.encode('utf-8')
		str_len = len(value)
		if str_len<=0xff:
			arr_out.append(4)
			arr_out.append(str_len)
			arr_out.fromstring(value)
		elif str_len>0xff and str_len<=0xffff:
			arr_out.append(5)
			arr_out.fromstring(struct.pack('!H', str_len))
			arr_out.fromstring(value)
		else:
			arr_out.append(6)
			arr_out.fromstring(struct.pack('!I', str_len))
			arr_out.fromstring(value)
	elif item_type==datetime.datetime:
		value = value.strftime('%Y-%m-%d %H:%M:%S')
		str_len = len(value)
		if str_len<=0xff:
			arr_out.append(4)
			arr_out.append(str_len)
			arr_out.fromstring(value)
		elif str_len>0xff and str_len<=0xffff:
			arr_out.append(5)
			arr_out.fromstring(struct.pack('!H', str_len))
			arr_out.fromstring(value)
		else:
			arr_out.append(6)
			arr_out.fromstring(struct.pack('!I', str_len))
			arr_out.fromstring(value)
	elif item_type==types.NoneType:
		arr_out.append(12)
	else:
		raise exceptions.TypeError,value

def _encode_dict(arr_out, dict):

	arr_out.append(0x08)
	item_count = len(dict)
	arr_out.fromstring(struct.pack('!I', item_count))

	for item in dict:

		value = dict[item]
		item_type = type(value)

		key_len = len(item)
		arr_out.append(key_len)
		arr_out.fromstring(item)

		if item_type==types.ListType or item_type==types.TupleType:
			_encode_list(arr_out, value)
		elif item_type==types.DictType:
			_encode_dict(arr_out, value)
		else:
			_encode_normal_type(arr_out, value)

def _encode_list(arr_out, l):

	arr_out.append(0x07)
	item_count = len(l)
	arr_out.fromstring(struct.pack('!I', item_count))
	for item in l:
		item_type = type(item)
		if item_type==types.ListType or item_type==types.TupleType:
			_encode_list(arr_out, item)
		elif item_type==types.DictType:
			_encode_dict(arr_out, item)
		else:
			_encode_normal_type(arr_out, item)


def main():
	if len(sys.argv) < 3:
		print sys.argv[0],"infile outfile"
		exit(0)
	f1 = open(sys.argv[1])
	f2 = open(sys.argv[2],'wb');

	data1= f1.read(-1);

	obj1 = {}
	obj1 = anyvalue_decode(data1)

	print obj1

	data2 = anyvalue_encode(obj1)
	f2.write(data2)

	obj2 = {}
	obj2 = anyvalue_decode(data2)

	print obj2

if __name__ == '__main__':
    main()
