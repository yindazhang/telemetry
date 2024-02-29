import os
import sys
import argparse

if __name__=="__main__":

    parser = argparse.ArgumentParser(description='')
    parser.add_argument('-f', dest='file', action='store', help="Specify the command file.")
    args = parser.parse_args()


    with open(args.file, 'r') as file:
        lines = file.readlines()
        for line in lines:
            print(line)
            os.system(line)
            os.system("sleep 10")
    
    print("Finish")