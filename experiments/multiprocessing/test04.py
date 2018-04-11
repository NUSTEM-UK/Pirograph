from multiprocessing import Pool, TimeoutError
import time
import os

def f(x):
    return x * x

if __name__ == '__main__':
    pool = Pool(processes=4)