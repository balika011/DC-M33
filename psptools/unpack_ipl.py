#!/usr/bin/env python3

import argparse
import struct
from psptool.kirk import kirk1

parser = argparse.ArgumentParser(description='IPL packer')

parser.add_argument('input', help='Input binary', type=str)
parser.add_argument('output', help='Output file', type=str)
parser.add_argument('--xor', help='xor key', type=lambda x : int(x, 0))

args = parser.parse_args()

with open(args.input, 'rb') as rf:
	with open(args.output, 'wb') as of:
		while True:
			block = bytearray(rf.read(0x1000))
			if not block:
				break;

			if args.xor:
				key = bytearray([0x8c, 0x64, 0xd0, 0x30, 0x44, 0x2d, 0xf5, 0x82, 0xb8, 0x72, 0x1e, 0xb4, 0x87, 0xcd, 0x24, 0xfd])

				for i in range(16):
					block[i] ^= key[i]
					
			block[0x62] = 0
					
			block = kirk1(block)
			
			address, size, entry, sum = struct.unpack('<IIII', block[:16])
			data = block[16:16+size]
			
			of.write(data)
