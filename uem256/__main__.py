from ._core import hash # import the hash function

if __name__ == "__main__": # declare 
    import sys
    if len(sys.argv) > 1:
        print(hash(sys.argv[1]))
    else:
        print(hash(""))
