#!/usr/bin/env python3

import argparse
import struct
from psptool.kirk import kirk1_encrypt_cmac, kirk4
from Crypto.Hash import SHA1

parser = argparse.ArgumentParser(description='IPL packer')

parser.add_argument('input', help='Input binary', type=str)
parser.add_argument('output', help='Output file', type=str)
parser.add_argument('--xor', help='Xor the header', action='store_true')

args = parser.parse_args()

def ipl_header(block_destination, size, entry, hash):
    return struct.pack('<IIII', block_destination, size, entry, hash)

with open(args.input, 'rb') as f:
    code = f.read()

header = ipl_header(0xBC10004C, 4, 0x10000005, 0) + struct.pack('<IIII', 0x32F6, 0, 0, 0) #reset exploit; 0x32F6 is USB_HOST, ATA_HDD, MS_1, ATA, USB, AVC, VME, SC, ME reset bit
block = header + code
key = bytearray.fromhex('AA'*16)
block_enc = kirk1_encrypt_cmac(block, aes_key=key, cmac_key=key)

sha_buf = block[8:8+12] + block[:8]

block_hash = SHA1.new(sha_buf).digest()
hash_padding = bytearray(12)
hash_enc = kirk4(block_hash + hash_padding, 0x6C)

padding = bytearray(0x1000 - len(block_enc) - 0x20) #pad with zeros to IPL block size minus the SHA1 hash at the end
block_enc = bytearray(block_enc + padding + hash_enc)

if args.xor:
	key = bytearray([0x8c, 0x64, 0xd0, 0x30, 0x44, 0x2d, 0xf5, 0x82, 0xb8, 0x72, 0x1e, 0xb4, 0x87, 0xcd, 0x24, 0xfd])

	for i in range(16):
		block_enc[i] ^= key[i]

with open(args.output, 'wb') as f:
    f.write(block_enc)
