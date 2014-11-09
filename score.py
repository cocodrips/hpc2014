import json
import pdb
with open('output.json', 'r') as f, open('scores/stage_score.txt', 'w') as w:
    data = json.loads(f.readline())
    for stage in data[1]:
        print stage[0][5]
        w.write("{}\n".format(stage[0][5]));

