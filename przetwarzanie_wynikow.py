import numpy as np
import matplotlib.pyplot as plt

def read_time(line):
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
            
        
filename = 'wszystkie_wyniki_bez_procesow_x_u_mnie.txt'
file = open(filename, "r")
lines = file.readlines()
team_nums = {}
times = []
num_of_teams = 15 # dla śmieci z calccollatzsolotimer i total
teams_nums = []
for i in range(num_of_teams):
    times.append([])
last_num = -1
for line in lines:
    i = 6
    name = ""
    while line[i] != '<':
        name = name + line[i]
        i = i+1
    if not(name in team_nums.keys()):
        last_num = last_num + 1
        team_nums[name] = last_num
        teams_nums.append(name)
        print("druzyna " + name + " dajemy jej numer " + str(last_num))
    czas = read_time(line)
    # print (team_nums[name])
    times[team_nums[name]].append(czas)
print(times)
times_numpy = []
for time in times:
    times_numpy.append(np.array(time))
print(times_numpy)
averages = []
for time in times_numpy:
    averages.append(np.average(time))
print(averages)

'''name_averages = {}
for i in range(1, num_of_teams):
    name_averages[team_nums[i]] = averages[i]
print(name_averages)'''

averages[11] = 0
averages[12] = 0 # żeby niezakodzone procesy z X nam nie psuły wykresów


plt.figure(figsize=(30, 3))
plt.bar(teams_nums[1:14], averages[1:14])
plt.suptitle('All teams avg my laptop')
plt.show()

plt.figure(figsize=(30, 3))
plt.bar(teams_nums[1:8], averages[1:8])
plt.suptitle('Not X teams avg my laptop')
plt.show()

plt.figure(figsize=(30, 3))
plt.bar(teams_nums[8:14], averages[8:14])
plt.suptitle('X teams avg my laptop')
plt.show()


