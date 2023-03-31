import os
from subprocess import Popen, PIPE

running = True
minimodem_call = ["minimodem", "--tx" , "-M", "1275", "-S", "1445", "-5", "--startbits", "1", "--stopbits", "1.5","50"]

def main():
    global running
    while running:
        line = input("> ")
        if line == "exit" or line == "EXIT":
            running = False
        
        line += "\n\r"
        p = Popen(minimodem_call, stdin=PIPE)
        p.stdin.write(bytes(line, 'utf-8'))
        p.stdin.close()
        p.wait()


if __name__ == "__main__":

    main()

