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
            
        

file = open('test1_timer_outputs.txt', "r")
lines = file.readlines()
team_nums = {}
times = []
num_of_teams = 14 # dla śmieci z calccollatzsolotimer
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
        print("druzyna " + name + " dajemy jej numer " + str(last_num))
    czas = read_time(line)
    # print (team_nums[name])
    times[team_nums[name]].append(czas)
print(times)
