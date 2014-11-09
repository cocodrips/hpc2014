import sys
argvs = sys.argv
argc = len(argvs)

if argc < 3:
    quit()

with open(argvs[1], 'r') as f1, open(argvs[2], 'r') as f2:
    i = 0
    sum1, sum2 = 0, 0
    
    for l1, l2 in zip(f1, f2):
        print i,
        i += 1
        l1, l2 = int(l1), int(l2)
        sum1 += l1
        sum2 += l2
        print "{:8d} {:8d}  {:8d}".format(l1, l2, l1- l2),
        
        if l1 - l2 < 0: 
            print "###"
        else:
            print 

    print "sum:   {:8d}  {:8d}".format(sum1, sum2)
