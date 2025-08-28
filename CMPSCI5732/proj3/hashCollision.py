from hashlib import sha256
from time import time_ns
from math import floor
import random

def generate_files():
    with open(file="small_file", mode="w+b") as small_file:
        small_file.write(random.randbytes(32))
    small_file.close()

    with open(file="large_file", mode="w+b") as large_file:
        large_file.write(random.randbytes(2048))
    large_file.close()

def hash_files(): 
    with open(file="small_file", mode="rb") as small_file:
        small_file_content = small_file.read()
    
    with open(file="large_file", mode="rb") as large_file:
            large_file_content = large_file.read()
    small_hash_total_time = 0
    for _ in range(0, 10000):
        start_time = floor(time_ns() / 1000)
        sha256(small_file_content)
        end_time = floor(time_ns() / 1000)
        small_hash_total_time += (end_time - start_time)
    small_file.close()
    small_hash_avg_time = small_hash_total_time / 100 

    large_hash_total_time = 0
    for _ in range(0, 10000):   
        start_time = floor(time_ns() / 1000)
        sha256(large_file_content)
        end_time = floor(time_ns() / 1000)
        large_hash_total_time += (end_time - start_time)
    large_file.close()
    large_hash_avg_time = large_hash_total_time / 100 

    print(f"Small Hash took an average of {small_hash_avg_time} microseconds")
    print(f"Large Hash took an average of {large_hash_avg_time} microseconds")

generate_files()
hash_files()
