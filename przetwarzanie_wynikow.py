def process_line(line):
    i = 6
    name = ""
    while line[i] != '<':
        name = name + line[i]
        i = i+1
    times[name].append(czas) # no i tu właśnie trzeba znaleźć czas
    print(name)

file = open('test1_timer_outputs.txt', "r")
lines = file.readlines()
times = {}
for line in lines:
    process_line(line, times)

