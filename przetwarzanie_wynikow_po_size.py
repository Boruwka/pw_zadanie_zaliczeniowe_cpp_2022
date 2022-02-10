import numpy as np
import matplotlib.pyplot as plt

def get_name(line):
    i = 6
    name = ""
    while line[i] != '<':
        name = name + line[i]
        i = i+1
    return name

def get_size(line):
    dodajemy = 0
    size = ""
    for z in line:
        if z == '<':
            dodajemy = 1
        elif z == '>':
            dodajemy = 0
            break
        elif dodajemy == 1:
            size = size + z
    size_num = int(size)
    return size_num 

def get_time(line):
    brudny_string = ""
    dodajemy = 0
    for z in line:
        if dodajemy and z != ' ':
            brudny_string = brudny_string + z
        if z == '=':
            dodajemy = 1
        if dodajemy and len(brudny_string) > 2 and z == ',':
            break
    # print(brudny_string)
    czysty_string = ""
    mnoznik = 1.
    for z in brudny_string:
        if z == 'm':
            mnoznik = 1.
            break
        elif z == 'u':
            mnoznik = 0.001
            break
        elif z == 's':
            mnoznik = 1000
            break
        else:
             czysty_string = czysty_string + z
    czas = float(czysty_string) * mnoznik
    # print (czas)
    return czas  

def get_wykres(processed_name, lines):
    sum_in_size = {}
    num_in_size = {}
    for line in lines:
        name = get_name(line)
        if name == processed_name:
            size = get_size(line)
            time = get_time(line)
            if size in sum_in_size.keys():
                sum_in_size[size] = sum_in_size[size] + time
            else:
                sum_in_size[size] = time
            if size in num_in_size.keys():
                num_in_size[size] = num_in_size[size] + 1
            else:
                num_in_size[size] = 1
            print("teraz w size " + str(size) + " jest " + str(num_in_size[size]) + " o sumie " + str(sum_in_size[size]))
    print("dla imienia " + processed_name)
    avg_list = []
    sizes_list = []
    for s in sum_in_size.keys():
        print("dodajemy size {s}")
        sizes_list.append(s)
        print("srednia w nim to " + str(sum_in_size[s]/num_in_size[s]))
        avg_list.append(sum_in_size[s]/num_in_size[s])
    avgs = np.array(avg_list)
    sizes = np.array(sizes_list)

    plt.figure(figsize=(30, 3))
    plt.bar(sizes, avgs)
    plt.suptitle(processed_name + 'my laptop avgs by size')
    plt.show()
            

filename = 'wszystkie_wyniki_bez_procesow_x_u_mnie.txt'
file = open(filename, "r")
lines = file.readlines()
processed_names = ['TeamNewThreads', 'TeamConstThreads', 'TeamPool', 'TeamNewProcesses', 'TeamConstProcesses', 'TeamNewThreadsX', 'TeamConstThreadsX', 'TeamPoolX', 'TeamNewProcessesX', 'TeamConstProcessesX']
for name in processed_names:
    get_wykres(name, lines)

