import os
import sys

if __name__=="__main__":

    with open('command.txt', 'r') as file:
        lines = file.readlines()
        for line in lines:
            print(line)
            os.system(line)
            os.system("sleep 10")
    
    print("Finish")