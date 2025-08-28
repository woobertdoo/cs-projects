from hashlib import sha256
from time import time_ns
from math import floor

def create_vanity():

    # Initialize 32 bytes of 0
    address = int.from_bytes(bytes(32))

    address_hash = sha256(address.to_bytes(length=32))
    # Hex strings start with 0x
    hash_str = address_hash.hexdigest()[2:]

    start_time = floor(time_ns() / 1000)

    while hash_str[0] != "0":
        address += 1
        address_hash = sha256(address.to_bytes(length=32))
        # Hex strings start with 0x
        hash_str = address_hash.hexdigest()[2:]

    end_time = floor(time_ns() / 1000)

    print(f"Time to generate 1 digit of vanity address: {end_time - start_time} microseconds")
    print(f"---------------")
    start_time = floor(time_ns() / 1000)

    while hash_str[0:2] != "05":
        address += 1
        address_hash = sha256(address.to_bytes(length=32))
        # Hex strings start with 0x
        hash_str = address_hash.hexdigest()[2:]

    end_time = floor(time_ns() / 1000)

    print(f"Time to generate 2 digit of vanity address: {end_time - start_time} microseconds")
create_vanity()
