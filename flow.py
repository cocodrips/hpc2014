import json
import pdb
with open('output.json', 'r') as f, open('flow.txt', 'w') as w:
    data = json.loads(f.readline())
    i = 0
    for stage in data[1]:
        print i, stage[0][4]
        w.write("{} {}\n".format(i, stage[0][4]));
        i+=1
