#!/usr/bin/python
countingmode = 1
longcnt = 1

import sys

print 'Number of arguments:', len(sys.argv), 'arguments.'
print 'Argument List:', str(sys.argv)


c = sys.argv[1]
r = sys.argv[2]
threshold = sys.argv[3]

for c in range(0, 8):
    for r in range(0, 8):
        matrix_file = open("matrix_px" + str(c) + "_" + str(r) + "_cnt_trim" + str(threshold) + ".cfg","w");

        matrix_file.write("#Peary CLICpix2 Matrix configuration\n")
        matrix_file.write("# ROW COL mask threshold countingmode testpulse longcnt\n")
        for row in range(0,128):
            for column in range(0, 128):
                mask = 1
                testpulse = 0
                #if row == int(r) and column == int(c):
                if (row%8) == int(r) and (column%8) == int(c):
                    print "Unmasked: " + str(column) + "," + str(row)
                    mask = 0
                    testpulse = 0
                matrix_file.write("%i %i %i %i %i %i %i\n" % (row, column, mask, int(threshold), countingmode, testpulse, longcnt))

        matrix_file.close()
