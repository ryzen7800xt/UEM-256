from ._core import hash

if __name__ == "__main__":
    import sys
    if len(sys.argv) > 1:
        print(hash(sys.argv[1]))
    else:
        print(hash(""))
