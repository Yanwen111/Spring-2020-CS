f = open("fakeCube_0_standard.txt", 'rb')
line = f.readline()
line = f.readline()
line = f.readline()
print(len(line))
for l in bytes(line):
    print(l)

f.close
