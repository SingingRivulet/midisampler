#include <map>
#include "../midiLoader.h"
#include "../midiMap.h"
#include "../sampler.h"
int main(int argc, char** argv) {
    bool processError = false;
    if (argc < 5) {
        return 0;
    }
    std::string input = argv[1];
    std::string output_ori = std::string(argv[2]);

    int noteChannel = atoi(argv[3]);

    std::cout << "输入:" << input << std::endl;
    std::cout << "旋律音轨:" << noteChannel << std::endl;
    std::cout << "和弦:" << argv[4] << std::endl;

    std::map<float, std::string> chordTimeMap;
    {
        auto fp = fopen(argv[4],"r");
        if(fp){
            char buf[256];
            int count = 0;
            while(!feof(fp)){
                bzero(buf,256);
                fgets(buf,256,fp);
                if(strlen(buf)>2){
                    std::istringstream iss(buf);
                    std::string bufs;
                    float buff;
                    iss>>bufs;
                    if(bufs=="0"){
                        bufs = "[]";
                    }
                    iss>>buff;
                    chordTimeMap[buff] = bufs;
                    ++count;
                }
            }
            printf("读取%d个和弦\n",count);
            fclose(fp);
        }
    }

    char buf_noteChannel[64];
    snprintf(buf_noteChannel, sizeof(buf_noteChannel), ".%d", noteChannel);

    mgnr::midiMap midiMap;
    mgnr::loadMidi(midiMap, input);
    midiMap.updateTimeMax();

    static const char* noteMap_1[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
    static const char* noteMap_2[] = {"C", "Db", "D", "Eb", "E", "F", "Gb", "G", "Ab", "A", "Bb", "B"};
    static const char* noteMap_3[] = {"Am", "A#m", "Bm", "Cm", "C#m", "Dm", "D#m", "Em", "Fm", "F#m", "Gm", "G#m"};
    static const char* noteMap_4[] = {"Am", "Bbm", "Bm", "Cm", "Dbm", "Dm", "Ebm", "Em", "Fm", "Gbm", "Gm", "Abm"};
    static const char* noteMap_5[] = {"a", "a#", "b", "c", "c#", "d", "d#", "e", "f", "f#", "g", "g#"};
    static const char* noteMap_6[] = {"a", "bb", "b", "c", "db", "d", "eb", "e", "f", "gb", "g", "ab"};
    static const char* noteMap_7[] = {"C", "D-", "D", "E-", "E", "F", "G-", "G", "A-", "A", "B-", "B"};
    static const char* noteMap_8[] = {"a", "b-", "b", "c", "d-", "d", "e-", "e", "f", "g-", "g", "a-"};
    int basetone = -1;
    if (argc >= 6) {
        for (int i = 0; i < 12; ++i) {
            if (
                strcmp(argv[5], noteMap_1[i]) == 0 ||
                strcmp(argv[5], noteMap_2[i]) == 0 ||
                strcmp(argv[5], noteMap_3[i]) == 0 ||
                strcmp(argv[5], noteMap_4[i]) == 0 ||
                strcmp(argv[5], noteMap_5[i]) == 0 ||
                strcmp(argv[5], noteMap_6[i]) == 0 ||
                strcmp(argv[5], noteMap_7[i]) == 0 ||
                strcmp(argv[5], noteMap_8[i]) == 0) {
                basetone = i;
                break;
            }
        }
        std::cout << "设置调性:";
    } else {
        std::cout << "识别调性:";
        basetone = midiMap.baseTone;
    }
    if(basetone==-1){
        std::cout << basetone << "(未知调性)" << std::endl;
        return 0;
    }
    std::cout << basetone << "(" << noteMap_1[basetone] << ")" << std::endl;

    int lenInBeat = (midiMap.noteTimeMax / midiMap.TPQ);
    std::vector<int> notes;
    std::vector<std::string> chords;
    char channel_note[64];
    char channel_chord[64];
    int numNote = 0;
    int numChord = 0;
    for (int i = 0; i < lenInBeat; i = i + 1) {
        auto chordIt = chordTimeMap.lower_bound(i);
        if (chordIt != chordTimeMap.end()) {
            chords.push_back(chordIt->second);
        } else {
            chords.push_back("[]");
        }
        ++numChord;
        for (int j = 0; j < 4; ++j) {
            notes.push_back(mgnr::sample(midiMap, i + j * 0.25, 0.25, basetone, [&](mgnr::note* n) {
                return (strstr(n->info.c_str(), buf_noteChannel) != NULL);
            }));
            ++numNote;
        }
    }
    //std::cout << "旋律:" << numNote << "和弦:" << numChord << std::endl;
    if (processError) {
        std::cout << "出现错误，停止写入" << std::endl;
        return 1;
    }
    std::cout << "旋律:" << notes.size() << "和弦:" << chords.size() << std::endl;
    auto fp = fopen(output_ori.c_str(), "a");
    if (fp) {
        if (argc >= 6) {
            fprintf(fp, "%s %s|[", input.c_str(),argv[5]);
        }else{
            fprintf(fp, "%s|[", input.c_str());
        }
        bool first = true;
        for (auto it : notes) {
            if (first) {
                fprintf(fp, "%d", it);
            } else {
                fprintf(fp, ",%d", it);
            }
            first = false;
        }
        fprintf(fp, "]|[");
        first = true;
        for (auto it : chords) {
            if (!first) {
                fprintf(fp, ",");
            }
            /*
            {
                fprintf(fp, "[");
                bool first = true;
                for (auto n : it) {
                    if (first) {
                        fprintf(fp, "%d", n);
                    } else {
                        fprintf(fp, ",%d", n);
                    }
                    first = false;
                }
                fprintf(fp, "]");
            }
            */
            fprintf(fp, "%s", it.c_str());
            first = false;
        }
        fprintf(fp, "]\n");
        fclose(fp);
    }
    return 0;
}
