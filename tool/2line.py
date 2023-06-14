from cgi import test
import os


def process(i, o):

    f = open(i)
    o = open(o, "w")

    line = f.readline()
    while line:
        pair = line.replace(" ", "").split("|")
        if len(pair) >= 3:
            chord = pair[2].lstrip("[\r\n").rstrip("]\r\n").split("],[")
            melody = pair[1].strip("[]").split(",")
            l_m = len(melody)
            l_c = len(chord)
            print("音符:"+str(l_m)+" 和弦:"+str(l_c))
            for i in range(l_c):
                m_arr = []
                for j in range(4):
                    try:
                        m_arr.append(melody[i*4+j])
                    except Exception:
                        m_arr.append("0")
                o.write("["+(",".join(m_arr))+"]|["+chord[i]+"]\n")
        line = f.readline()


# for i in range(310, 400, 10):
#    path = "test_ckpt"+str(i)+".txt"
#    print(path)
#    process("./tf310-390/"+path, "./tf310-390-2line/"+path)

# for dirpath, dirnames, filenames in os.walk("/home/ubuntu/project/chcpy/outputs/chordplay/genChord"):
#    for filepath in filenames:
#        inpath = os.path.join(dirpath,filepath)
#        outpath = "/home/ubuntu/project/chcpy/outputs/chordplay/genChord2l/"+filepath
#        process(inpath,outpath)

process("test/mld_chord_mf/海阔天空.txt", "test/mld_chord_mf/海阔天空.2line.txt")
process("test/mld_chord_mf/可能.txt", "test/mld_chord_mf/可能.2line.txt")
process("test/mld_chord_mf/平凡之路.txt", "test/mld_chord_mf/平凡之路.2line.txt")
process("test/mld_chord_mf/十年.txt", "test/mld_chord_mf/十年.2line.txt")
process("test/mld_chord_mf/晚风心里吹.txt", "test/mld_chord_mf/晚风心里吹.2line.txt")
process("test/mld_chord_mf/喜欢你.txt", "test/mld_chord_mf/喜欢你.2line.txt")
